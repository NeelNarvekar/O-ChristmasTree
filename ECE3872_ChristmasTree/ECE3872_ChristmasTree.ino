#include <Adafruit_NeoPixel.h>
#include <Servo.h>
//#include <Tone.h>

// Digital Pin Definitions
#define SONAR_ECHO    0
#define SONAR_TRIG    1
#define INTERRUPT_1   2 // RESET * PLAY
#define INTERRUPT_2   3 // RESET * RECORD
 // space for 4?
#define STATE_LED     5
#define SERVO_2_4     6
#define RESET_LED     7
 // space for 8! 
 // space for 9!
#define SERVO_1_3     10
#define SPEAKER       11
#define STOP          12
#define LED           13 // Neopixel LED strip
#define SPDT_PLAYMODE 14

//#define TESTLED1      5
//#define TESTLED2      7

// Values
#define LIVE          0
#define RECORDING     1
#define NOTES_STORED  100
#define NUM_LEDS      19
#define MAX_DISTANCE  20.0 // Furthest distance the Sonar sensor will read (cm)


// Note frequencies with their int storare mapping (#)
// https://www.liutaiomottola.com/formulae/freqtab.htm
#define A2       110.000  // (1)
#define As2      116.541  // (2)
#define B2       123.471  // (3)
#define C3       130.813  // (4)
#define Cs3      138.591  // (5)
#define D3       146.832  // (6)
#define Ds3      155.563  // (7)
#define E3       164.814  // (8)
#define F3       174.614  // (9)
#define Fs3      184.997  // (10)
#define G3       195.998  // (11)
#define Gs3      207.652  // (12)

// Integer Frequencies
#define A2i      110  // (1)
#define As2i     117  // (2)
#define B2i      123  // (3)
#define C3i      131  // (4)
#define Cs3i     139  // (5)
#define D3i      147  // (6)
#define Ds3i     156  // (7)
#define E3i      165  // (8)
#define F3i      175  // (9)
#define Fs3i     185  // (10)
#define G3i      196  // (11)
#define Gs3i     208  // (12)

const float freqs[] = { A2, As2, B2, C3, Cs3, D3, Ds3, E3, F3, Fs3, G3, Gs3 };
const int freqs_i[] = { A2i, As2i, B2i, C3i, Cs3i, D3i, Ds3i, E3i, F3i, Fs3i, G3i, Gs3i };

// Global variables

volatile int state_var_0;
volatile int state_var_1;
bool on = false;
unsigned long tstart;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, LED, NEO_GRB + NEO_KHZ800);

Servo myservo_1_3;
Servo myservo_2_4;

// Recording and playing back different notes
volatile int recorded_notes[NOTES_STORED];
volatile unsigned int idx_record = 0;
volatile unsigned int idx_play = 0; // Check that this idx is in bounds before playing!
volatile bool is_recorded_data = false;
volatile bool is_space_remaining = true;

int servoPos_1_3 = 90;
int servoPos_2_4 = 90;

// NOTE! Arduino Nano only has 2,3 as interrupt pins...

void setup() {
  // Set GPIO pins to their proper mode
  myservo_1_3.attach(SERVO_1_3);
  myservo_2_4.attach(SERVO_2_4);
  strip.begin();
  strip.clear();
//  Serial.begin(115200);
  pinMode(STOP, INPUT);
  pinMode(SONAR_TRIG, OUTPUT);
  pinMode(SONAR_ECHO, INPUT);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(STATE_LED, OUTPUT);
  pinMode(RESET_LED, OUTPUT);

  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(STATE_LED, LOW);
  digitalWrite(RESET_LED, LOW);
  
  pinMode(SPDT_PLAYMODE, INPUT);
  attachInterrupt(digitalPinToInterrupt(INTERRUPT_1), updateStateVar1, CHANGE);
  attachInterrupt(digitalPinToInterrupt(INTERRUPT_2), updateStateVar2, CHANGE);
  
  eraseRecording();
  updateStateVar1(); 
  updateStateVar2();

  myservo_1_3.write(servoPos_1_3);
  myservo_2_4.write(servoPos_2_4);

}

void loop() {
  // Main program loop to run infinitely
  stateMachineHandler();
}

/* Update State Var 1
 *  
 * Interrupt triggered when rotary switch turned or reset button pressed, updates state and resets state LEDs
 *  RESET * PLAY
 *
 * Params: None
 * Returns: None
 */
void updateStateVar1() {
  state_var_0 = digitalRead(INTERRUPT_1);
}

/* Update State Var 2
 *  
 * Interrupt triggered when rotary switch turned or reset button pressed, updates state and resets state LEDs
 *  RESET * RECORD
 *
 * Params: None
 * Returns: None
 */
void updateStateVar2() {
  state_var_1 = digitalRead(INTERRUPT_2);
}

/* State Machine Handler
 *  
 *  State Machine handler, constantly updating current state based on interrupt variables
 *
 * Params: None
 * Returns: None
 */
void stateMachineHandler() {
  // Reads variables corresponding to INT1=REST*PLAY and INT2=RESET*RECORD 
  if (state_var_0 == 0) {
    if (state_var_1 == 0) {
      //Serial.println("RESET STATE 0 0");
      reset(); // 0 0 -> RESET STATE
    } else {
      //Serial.println("PLAY STATE 0 1");
      rotaryPlay(); // 0 1 -> PLAY STATE
    }
  } else {
    if (state_var_1 == 0) {
      //Serial.println("RECORD STATE 1 0");
      rotaryRecord(); // 1 0 -> RECORD STATE
    } else {
      if (digitalRead(STOP) == 0) {
        //Serial.println("STOP STATE 1 1");
        rotaryStop(); // 1 1 -> STOP STATE
      }
    }
  }
}

// ------ STATE FUNCTIONS -------

/* Rotary Record 
 *  
 * 
 * State Machine stop function triggered when rotary switch set to record
 *
 * Params: None
 * Returns: None
 */
void rotaryRecord() {
  // Blink when recording, except if out of space then turn off LED
  while (state_var_0 == 1 && state_var_1 == 0) { // enter while loop if room to record
    tstart = millis();
    digitalWrite(STATE_LED, is_space_remaining ? HIGH : LOW);
    recordAudioData();
    // Wait till 250ms have passed to turn state LED off for blinking
    while (state_var_0 == 1 && state_var_1 == 0 && millis() <= tstart + 250);
    digitalWrite(STATE_LED, LOW); // Blink LED
    while (state_var_0 == 1 && state_var_1 == 0 && millis() <= tstart + 500);
  }
  digitalWrite(STATE_LED, LOW);
  is_recorded_data = true;
}


/* Rotary Stop 
 *  
 * 
 * State Machine stop function triggered when rotary switch set to stop
 *
 * Params: None
 * Returns: None
 */
void rotaryStop() {
  ////Serial.print("STOP STATE\n");
  digitalWrite(STATE_LED, LOW);
  digitalWrite(LED_BUILTIN, LOW);
  while (state_var_0 == 1 && state_var_1 == 1) {
     // Should we do anything in stop?
  }
}


/* Rotary Play 
 *  
 * 
 * State Machine play function triggered when rotary switch set to play
 *
 * Params: None
 * Returns: None
 */
void rotaryPlay() {
  ////Serial.print("PLAY STATE\n");
  // OUTER LOOP OCCURS EVERY 500 MILLISECONDS
  while (state_var_0 == 0 && state_var_1 == 1) {
    // Get start time and note
    tstart = millis();
    digitalWrite(STATE_LED, HIGH);
    int mode = digitalRead(SPDT_PLAYMODE); 
    if (play(mode) == -1) digitalWrite(STATE_LED, LOW);
    // Spin until state change or time for next note
    while (state_var_0 == 0 && state_var_1 == 1 && millis() <= tstart + 500);
  }
  // Exit state code - turn off LED and stop the tone
  digitalWrite(STATE_LED, LOW);
  noTone(SPEAKER);
}


/* Reset 
 *  
 * 
 * State Machine reset function triggered when reset button pressed
 *
 * Params: None
 * Returns: None
 */
void reset() {
  tstart = millis();
  digitalWrite(RESET_LED, HIGH);
  idx_play = 0;
  
  // Write the servos back to 90
  int finalPos_1_3 = 90;
  int finalPos_2_4 = 90;
  int step_1_3 = (finalPos_1_3 - servoPos_1_3)/10;
  int step_2_4 = (finalPos_2_4 - servoPos_2_4)/10;
  for (int i = 0; i < 9; ++i) {
    servoPos_1_3 += step_1_3;
    myservo_1_3.write(servoPos_1_3);
    delay(10);
    servoPos_2_4 += step_2_4;
    myservo_2_4.write(servoPos_1_3);
    delay(10);
  }
  servoPos_1_3 = finalPos_1_3;
  myservo_1_3.write(servoPos_1_3);
  delay(10);
  servoPos_2_4 = finalPos_2_4;
  myservo_2_4.write(servoPos_1_3);

  // Delay to ensure the LEDs don't spike the power
  delay(25);
  for (int i = 0; i < NUM_LEDS; ++i) {
    strip.setPixelColor(i, 32 + i * 4, 168, 228 - 4 * i);
  }
  strip.show();
  //Serial.print("RESET STATE\n");
  while (state_var_0 == 0 && state_var_1 == 0) {
    // if reset held down for > 3s, recording erased
    if (is_recorded_data && millis() >= tstart + 3000) { 
      //Serial.print("ERASING RECORDING\n");
      eraseRecording();
      // Blink 3 times to show recording erased
      digitalWrite(RESET_LED, LOW);
      delay(100);
      digitalWrite(RESET_LED, HIGH);
      delay(100);
      digitalWrite(RESET_LED, LOW);
      delay(100);
      digitalWrite(RESET_LED, HIGH);
      delay(100);
      digitalWrite(RESET_LED, LOW);
      delay(100);
      digitalWrite(RESET_LED, HIGH);
    }
  }
  // Exit state cleanup - turn off Reset LED
  digitalWrite(RESET_LED, LOW);
}




// ----------- HELPER FUNCTIONS -----------

/* Erase Recording
 * 
 *  Clears the recorded notes array and resets the record/play indices
 * 
 * Params: None
 * Returns: None
 */
void eraseRecording() {
  idx_record = 0;
  idx_play = 0;
  is_recorded_data = false;
  is_space_remaining = true;
  for (int i = 0; i < NOTES_STORED; ++i) {
    recorded_notes[i] = 0;
  }
}


/* Read Sonar cm
 *  
 *  Blocking call to measure distance on sonar 
 *  
 * Params: None
 * Returns: Distance (in cm) measured from front of sonar sensor
 */
unsigned long read_sonar_cm() {
  digitalWrite(SONAR_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(SONAR_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(SONAR_TRIG, LOW);
  unsigned long cm = pulseIn(SONAR_ECHO, HIGH, 300000) / 58;
  return cm; // Distance in cm
}

/* Get Note Index - int
 * 
 * Reads and scaled Sonar distance input to determine index of desired note
 * Note determined on continuous scale before conversion to note
 * 
 * 
 * Params: None
 * Returns: int frequency - the rounded frequency value
 */
int getNote() {
  unsigned long cm = read_sonar_cm();
  int note = (cm-2)/3;
  if (note < 1 || note > 12) note = -1;
  return note;
}

/* Get Frequency - int
 * 
 * Reads and scaled Sonar distance input to determine frequency of desired note
 * Frequency determined on continuous scale before conversion to note
 * 
 * 
 * Params: None
 * Returns: int frequency - the rounded frequency value
 */
inline int getFrequency_i() {
  return freqs_i[getNote()];
}


/* Get Frequency - float
 * 
 * Reads and scaled Sonar distance input to determine frequency of desired note
 * Frequency determined on continuous scale before conversion to note
 * 
 * 
 * Params: None
 * Returns: float frequency - the raw frequency value
 */
float getFrequency() {
  // This function scales notes/frequencies in geometric fashion like a guitar string
  //  Should it be changed to provide a linear relationship between distance and note?
  float freq = 107.0 * (((float) read_sonar_cm()) / MAX_DISTANCE) + 108.0;
  return freq;
}


/* Get Note From Freqency
 * 
 * Maps a given frequency to its nearest output note
 *  
 * Params: float frequency - raw frequency data
 * Returns: The integer mapping of the nearest note to the input frequncy
 */
int getNoteFromFrequency(float freq) {
    // Separate frequencies into bins with cutoffs as geometric mean of two frequencies
    if (freq < 113.223) return 1;
    else if (freq < 119.956) return 2;
    else if (freq < 127.089) return 3;
    else if (freq < 134.646) return 4;
    else if (freq < 142.652) return 5;
    else if (freq < 151.134) return 6;
    else if (freq < 160.122) return 7;
    else if (freq < 169.643) return 8;
    else if (freq < 179.731) return 9;
    else if (freq < 190.418) return 10;
    else if (freq < 201.741) return 11;
    else if (freq < 213.737) return 12;
    // If the frequency is greater than the cutoff, don't play anything.
    return -1; 
}


/* Get Frequency from Note
 * 
 * Provides the frequency of a note's mapping
 *  Use with indices of notes as defined up top (starting with 1)
 *  
 * Params: int note - encoded note data
 * Returns: frequency corresponding to that note
 */
inline float getFrequencyFromNote(int note) {
  return freqs[note-1];
}


/* Get Next Recorded Note
 * 
 * Gives the next note in the recording (if applicable) and increments the playback index
 * 
 * Params: None
 * Returns: Integer encoding of the next recorded note for playback
 */
int getNextRecordedNote() {
  if (idx_play < idx_record && idx_play < NOTES_STORED) {
    int note = recorded_notes[idx_play++ % idx_record];
    idx_play = idx_play == idx_record ? idx_play = 0 : idx_play;
    return note;
  }  else {
    return -1; // Error code - no more data
  }
}


/* Set Next Recorded Note
 * 
 * Writes the next recorded note to the note storage array
 * 
 * Params: int note - the next note to record
 * Returns: int status - 0 if successful, -1 if out of recording space
 */
int setNextRecordedNote(int note) {
  if (idx_record < NOTES_STORED) {
    recorded_notes[idx_record++] = note;
    return 0;
  }
    is_space_remaining = false;
    return -1; // Error code
}


/* Play
 *  
 * Plays notes from speaker live or from recording based on mode
 * 
 * Params: int mode - read from the slide switch controlling play live or play from recording
 * Returns: Int note - the note that it detected and played
 */
int play(int mode) {
  int note;
  // Select note from either a recording or live input
  if (mode == LIVE) {
    // Live
    note = getNote();
  } else {
    // Recording
    note = getNextRecordedNote();
  } 
  //Serial.print("Play: Note ");
  //Serial.println(note);
  if (note == -1) return note; // Don't play anything if no note is returned
  lightLEDs(note);
  delay(10);
  speaker(note); 
  moveMotors(note);
  return note;
}


/* Record Audio Data
 *  
 * SIMULATION: Records notes in sequence of 1-12
 *  
 * Calculates current frequency, retrieves the note, and stores it
 * 
 * Params: None
 * Returns: int storageStatus: 0 if all clear, -1 if out of space
 */
int recordAudioData() {
  int note = play(LIVE);
  if (note > -1 && setNextRecordedNote(note) == -1) {
     return -1;
  }
  return 0;
}

/* Move Motors
 *  
 *  
 * Moves both servo motors to a set position based on the note
 * 
 * Params: int note - the next note to move servos based on
 * Returns: None
 */
void moveMotors(int note) {
//  //Serial.print("Move Motors\n");
  // Move 
  int finalPos_1_3 = 25+10*note;
  int finalPos_2_4 = 155-10*note;
  int step_1_3 = (finalPos_1_3 - servoPos_1_3)/10;
  int step_2_4 = (finalPos_2_4 - servoPos_2_4)/10;
  for (int i = 0; i < 9; ++i) {
    servoPos_1_3 += step_1_3;
    myservo_1_3.write(servoPos_1_3);
    delay(10);
    servoPos_2_4 += step_2_4;
    myservo_2_4.write(servoPos_1_3); // Changed from 2-4 to 1-3
    delay(10);
  }
  servoPos_1_3 = finalPos_1_3;
  myservo_1_3.write(servoPos_1_3);
  delay(10);
  servoPos_2_4 = finalPos_1_3;
  myservo_2_4.write(servoPos_1_3); // Changed from 2-4 to 1-3
    switch(note) {
    case 1: 
      break;
    case 2: 
      break;
    case 3: 
      break;
    case 4: 
      break;
    case 5: 
      break;
    case 6: 
      break;
    case 7: 
      break;
    case 8: 
      break;
    case 9: 
      break;
    case 10: 
      break;
    case 11: 
      break;
    case 12: 
      break;
     default:
      myservo_1_3.write(90);
      myservo_2_4.write(90);
      break;
    }
  return;
}

/* Speaker 
 *  
 * 
 * Params: int note - the next note to play on the speaker
 * Returns: None
 */
// #inline-always
void speaker(int note) {
  tone(SPEAKER, getFrequencyFromNote(note), 500);
}

/* Light LEDs 
 *  
 * 
 * Params: int note - the next note to change LED colors based on
 * Returns: None
 */
void lightLEDs(int note) {
  //Serial.print("LEDs: note ");
  //Serial.print(note);
  //Serial.print("\n");
  strip.clear();
  for (int i = 0; i < NUM_LEDS; ++i) {
    // GRB
    strip.setPixelColor(i, (note + i % 2) * 200 + 6*(i % 8), ((note+i+1) % 2)*200 + 12*(i % 4), 7*(i % 16));
  }
  strip.show();
  /*
  switch(note) {
    case 1: 
      break;
    case 2: 
      break;
    case 3: 
      break;
    case 4: 
      break;
    case 5: 
      break;
    case 6: 
      break;
    case 7: 
      break;
    case 8: 
      break;
    case 9: 
      break;
    case 10: 
      break;
    case 11: 
      break;
    case 12: 
      break;
     default:
      strip.clear();
      break;
  } //*/
  strip.show();
}
