#include <Adafruit_NeoPixel.h>
#include <Servo.h>
//#include <Tone.h>

// Digital Pin Definitions
#define SONAR_ECHO    0
#define SONAR_TRIG    1
#define INTERRUPT_1   2 // RESET * PLAY
#define INTERRUPT_2   3 // RESET * RECORD
 // space for 4?
#define RESET_LED     5
#define SERVO_1_3     6
#define STATE_LED     7
 // space for 8! 
 // space for 9!
#define SERVO_2_4     10
#define SPEAKER       11
#define STOP          12
#define LED           13 // Neopixel LED strip
#define SPDT_PLAYMODE 14

#define TESTLED1      5
#define TESTLED2      7

// Values
#define LIVE          1
#define RECORDING     0
#define NOTES_STORED  100
#define NUM_LEDS      50
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

// NOTE! Arduino Nano only has 2,3 as interrupt pins...

void setup() {
  // Set GPIO pins to their proper mode
  myservo_1_3.attach(SERVO_1_3);
  myservo_2_4.attach(SERVO_2_4);
  strip.begin();
  strip.clear();
  Serial.begin(115200);
  pinMode(STOP, INPUT);
//  pinMode(STATE_LED, OUTPUT);
  pinMode(SONAR_TRIG, OUTPUT);
  pinMode(SONAR_ECHO, INPUT);

  pinMode(LED_BUILTIN, OUTPUT);

  pinMode(TESTLED1, OUTPUT);
  pinMode(TESTLED2, OUTPUT);
  
  pinMode(SPDT_PLAYMODE, INPUT);
  attachInterrupt(digitalPinToInterrupt(INTERRUPT_1), updateStateVar1, CHANGE);
  attachInterrupt(digitalPinToInterrupt(INTERRUPT_2), updateStateVar2, CHANGE);
  
  eraseRecording();
  updateStateVar1(); 
  updateStateVar2();

//  Serial.print()

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
//  digitalWrite(STATE_LED, LOW);
//  digitalWrite(RESET_LED, LOW);
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
//  digitalWrite(STATE_LED, LOW);
//  digitalWrite(RESET_LED, LOW);
}

/* State Machine Handler
 *  
 *  State Machine handler, constantly updating current state based on interrupt variables
 *
 * Params: None
 * Returns: None
 */
void stateMachineHandler() {
  digitalWrite(LED_BUILTIN, LOW);
  // Reads variables corresponding to INT1=REST*PLAY and INT2=RESET*RECORD 
  if (state_var_0 == 0) {
    if (state_var_1 == 0) {
      //Serial.println("RESET STATE 0 0");
      reset(); // 0 0 -> RESET STATE
      ////Serial.print("0 0\n");
    } else {
      //Serial.println("PLAY STATE 0 1");
      rotaryPlay(); // 0 1 -> PLAY STATE
      ////Serial.print("0 1\n");
    }
  } else {
    if (state_var_1 == 0) {
      //Serial.println("RECORD STATE 1 0");
      rotaryRecord(); // 1 0 -> RECORD STATE
        ////Serial.print("1 0\n");
    } else {
      if (digitalRead(STOP) == 0) {
        //Serial.println("STOP STATE 1 1");
        rotaryStop(); // 1 1 -> STOP STATE
        ////Serial.print("1 1\n");
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
  ////Serial.print("RECORD STATE\n");
  digitalWrite(TESTLED1, HIGH);
  digitalWrite(TESTLED2, LOW);
  // if out of space then hold solid LED
  while (state_var_0 == 1 && state_var_1 == 0) { // enter while loop if room to record
//    digitalWrite(STATE_LED, HIGH);
    digitalWrite(LED_BUILTIN, HIGH);
    recordAudioData();
//    digitalWrite(STATE_LED, LOW);
  }
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
  digitalWrite(TESTLED1, HIGH);
  digitalWrite(TESTLED2, HIGH);
  ////Serial.print("STOP STATE\n");
  while (state_var_0 == 1 && state_var_1 == 1) {
    digitalWrite(LED_BUILTIN, HIGH);
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
  digitalWrite(TESTLED1, LOW);
  digitalWrite(TESTLED2, HIGH);
  while (state_var_0 == 0 && state_var_1 == 1) {
    digitalWrite(LED_BUILTIN, HIGH);
    // Get start time and note
    tstart = millis();
//    digitalWrite(STATE_LED, HIGH);
    int mode = digitalRead(SPDT_PLAYMODE); 
    play(mode); // Non-blocking!
    // Spin until state change or time for next note
    while (state_var_0 == 0 && state_var_1 == 1 && millis() <= tstart + 500);
  }
  // Exit state code - turn off LED and stop the tone
//  digitalWrite(STATE_LED, LOW);
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
  digitalWrite(TESTLED1, LOW);
  digitalWrite(TESTLED2, LOW);
//  digitalWrite(RESET_LED, HIGH);
  //Serial.print("RESET STATE\n");
  while (state_var_0 == 0 && state_var_1 == 0) {
    digitalWrite(LED_BUILTIN, HIGH);
    // if reset held down for > 3s, recording erased
    if (is_recorded_data && millis() >= tstart + 3000) { 
      //Serial.print("ERASING RECORDING\n");
      eraseRecording();
//      digitalWrite(RESET_LED, LOW);
      delay(100);
//      digitalWrite(RESET_LED, HIGH);
    }
  }
  // Exit state cleanup
//  digitalWrite(RESET_LED, LOW);
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
//  //Serial.print(cm);
//  //Serial.print(" cm - ");
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
//  int note = (read_sonar_cm()- 2)/3;
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
  // TODO: this function scales notes/frequencies in geometric fashion like a guitar string
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
    ////Serial.print(idx_record);
    return 0;
  }
    return -1; // Error code
}


/* Play
 *  
 * Plays notes from speaker live or from recording based on mode
 * 
 * Params: int mode - read from the slide switch controlling play live or play from recording
 * Returns: None
 */
void play(int mode) {
  int note;
  mode = LIVE;
  // Select note from either a recording or live input
  if (mode == LIVE) {
    // Live
    note = getNote();
//    note = getNoteFromFrequency(getFrequency());
  } else {
    // Recording
    note = getNextRecordedNote();
  } 
  //Serial.print("Play: Note ");
  //Serial.println(note);
  if (note == -1) return; // Don't play anything if no note is returned
  moveMotors(note); 
  lightLEDs(note);
  speaker(note); 
}


/* Record Audio Data
 *  
 * SIMULATION: Records notes in sequence of 1-12
 *  
 * Calculates current frequency, retrieves the note, and stores it
 * 
 * Params: None
 * Returns: None
 */
void recordAudioData() {
  // Uncomment vvv these vvv for actual hardware
  float freq = getFrequency();
  int note = getNoteFromFrequency(freq);
  // SIMULATION BEHVAIOIR: Record notes in repeating sequence 1-12
  //int note = idx_record % 12 + 1;
  //Serial.print("Record: ");
  //Serial.print(note);
  //Serial.print("\n");
  if (setNextRecordedNote(note) == -1) {
     //TODO: what to do if you're out of space?
  }
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
  myservo_1_3.write(82.5+7.5*note);
  myservo_2_4.write(97.5-7.5*note);
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
      myservo_1_3.write(0);
      myservo_2_4.write(0);
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
  //int volumeReading = analogRead(POT);
  //byte pwm = map(volumeReading, 0, 1024, 0, 220);
  //analogWrite(VOL, pwm);
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
    strip.setPixelColor(i, note * 24, 15*(i % 15), 255 - 4*i - 4 * note);
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
