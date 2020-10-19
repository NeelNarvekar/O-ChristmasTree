#include <Adafruit_NeoPixel.h>
#include <Servo.h>

// Digital Pin Definitions
#define SONAR_ECHO    0
#define SONAR_TRIG    1
#define INTERRUPT_1   2 // RESET * PLAY
#define INTERRUPT_2   3 // RESET * RECORD
#define SPDT_PLAYMODE 4
#define RESET_LED     5
#define SERVO_1_3     6
#define STATE_LED     7
 // space for 8!
#define VOL           9 // this will eventually be analog, connected to speaker
#define SERVO_2_4     10
 // space for 11!
#define STOP          12
#define LED           13 // Neopixel LED strip


// Values
#define LIVE          1
#define RECORDING     0
#define NOTES_STORED  100
#define NUM_LEDS      20
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

const float freqs[] = { A2, As2, B2, C3, Cs3, D3, Ds3, E3, F3, Fs3, G3, Gs3 };

// Global variables

volatile int state_var_0;
volatile int state_var_1;
bool on = false;
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
  myservo_1_3.attach(6);
  myservo_2_4.attach(10);
  strip.begin();
  strip.show();
  Serial.begin(9600);
  pinMode(STOP, INPUT);
  pinMode(STATE_LED, OUTPUT);
  pinMode(SONAR_TRIG, OUTPUT);
  pinMode(SONAR_ECHO, INPUT);
  
  pinMode(SPDT_PLAYMODE, INPUT);
  attachInterrupt(digitalPinToInterrupt(INTERRUPT_1), updateStateVar1, CHANGE);
  attachInterrupt(digitalPinToInterrupt(INTERRUPT_2), updateStateVar2, CHANGE);
  
  eraseRecording();
  updateStateVar1(); 
  updateStateVar2();

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
  digitalWrite(STATE_LED, LOW);
  digitalWrite(RESET_LED, LOW);
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
  digitalWrite(STATE_LED, LOW);
  digitalWrite(RESET_LED, LOW);
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
      reset(); // 0 0 -> RESET STATE
      //Serial.print("0 0\n");
    } else {
      rotaryPlay(); // 0 1 -> PLAY STATE
      //Serial.print("0 1\n");
    }
  } else {
    if (state_var_1 == 0) {
      rotaryRecord(); // 1 0 -> RECORD STATE
        //Serial.print("1 0\n");
    } else {
      if (digitalRead(STOP) == 0) {
        rotaryStop(); // 1 1 -> STOP STATE
        //Serial.print("1 1\n");
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
  Serial.print("RECORD STATE\n");
  // if out of space then hold solid LED
  while (state_var_0 == 1 && state_var_1 == 0) { // enter while loop if room to record
    digitalWrite(STATE_LED, HIGH);
    recordAudioData();
    delay(240); // these will mess up recording of notes. Once recordAudioData() is populated, should take enough time to replicate blink effect
    digitalWrite(STATE_LED, LOW);
    delay(240);
  }
  is_recorded_data = true;
  stateMachineHandler();
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
  Serial.print("STOP STATE\n");
  while (state_var_0 == 1 && state_var_1 == 1) {
     // Should we do anything in stop?
  }
  stateMachineHandler();
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
  Serial.print("PLAY STATE\n");
  while (state_var_0 == 0 && state_var_1 == 1) {
    digitalWrite(STATE_LED, HIGH);
    int mode = digitalRead(SPDT_PLAYMODE); 
    play(mode); // based on mode will play live or from recording
  }
  digitalWrite(STATE_LED, LOW);
  stateMachineHandler();
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
  Serial.print("RESET STATE\n");
  unsigned long time_start = millis();
  while (state_var_0 == 0 && state_var_1 == 0) {
    digitalWrite(RESET_LED, HIGH);
    if (millis() - time_start >= 3000 && is_recorded_data) { // if reset held down for > 3s, recording erased
      Serial.print("ERASING RECORDING\n");
      eraseRecording();
    }
    delay(100);
    digitalWrite(RESET_LED, LOW);
    delay(100);
  }
  stateMachineHandler();
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
  for (int i = 0; i < NOTES_STORED; ++i) {
    recorded_notes[i] = 0;
  }
  idx_record = 0;
  idx_play = 0;
  is_recorded_data = false;
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
//  Serial.print(cm);
//  Serial.print(" cm - ");
  return cm; // Distance in cm
}


/* Get Frequency
 * 
 * Reads and scaled Sonar distance input to determine frequency of desired note
 * Frequency determined on continuous scale before conversion to note
 * 
 * 
 * Params: None
 * Returns: int frequency - the raw frequency value TOD
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
    //Serial.print(idx_record);
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
  if (mode == LIVE) {
    // Live
    note = getNoteFromFrequency(getFrequency());
  } else {
    // Recording
    note = getNextRecordedNote();
  } 
  if (note == -1) return; // Don't play anything if no note is returned
  moveMotors(note); 
  lightLEDs(note);
  speaker(note);
  delay(500); // TODO: update this timing logic to account for variable code delay 
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
  Serial.print("Record: ");
  Serial.print(note);
  Serial.print("\n");
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
  Serial.print("Move Motors\n");
  myservo_1_3.write(82.5+7.5*note);
  myservo_2_4.write(97.5-7.5*note);
  /*
  if (note == 1) { // A 
    myservo_1_3.write(90);
    myservo_2_4.write(90);
    //Serial.print("Move note: 1\n");
  } else if (note == 2) { // A#
    myservo_1_3.write(97.5);
    myservo_2_4.write(82.5);
    //Serial.print("Move note: 2\n");
  } else if (note == 3) { // B
    myservo_1_3.write(105);
    myservo_2_4.write(75);
    //Serial.print("Move note: 3\n");
  } else if (note == 4) { // C
    myservo_1_3.write(112.5);
    myservo_2_4.write(67.5);
    //Serial.print("Move note: 4\n");
  } else if (note == 5) { // C#
    myservo_1_3.write(120);
    myservo_2_4.write(60);
    //Serial.print("Move note: 5\n");
  } else if (note == 6) { // D
    myservo_1_3.write(127.5);
    myservo_2_4.write(52.5);
    //Serial.print("Move note: 6\n");
  } else if (note == 7) { // D#
    myservo_1_3.write(135);
    myservo_2_4.write(45);
    //Serial.print("Move note: 7\n");
  } else if (note == 8) { // E
    myservo_1_3.write(142.5);
    myservo_2_4.write(37.5);
    //Serial.print("Move note: 8\n");
  } else if (note == 9) { // F
    myservo_1_3.write(150);
    myservo_2_4.write(30);
    //Serial.print("Move note: 9\n");
  } else if (note == 10) { // F
    myservo_1_3.write(157.5);
    myservo_2_4.write(22.5);
    //Serial.print("Move note: 10\n");
  } else if (note == 11) { // G
    myservo_1_3.write(165);
    myservo_2_4.write(15);
    //Serial.print("Move note: 11\n");
  } else if (note == 12) { // G#
    myservo_1_3.write(172.5);
    myservo_2_4.write(7.5);
    //Serial.print("Move note: 12\n");
  } */
  return;
}

/* Speaker 
 *  
 * 
 * Params: int note - the next note to play on the speaker
 * Returns: None
 */
void speaker(int note) {
  //int volumeReading = analogRead(POT);
  //byte pwm = map(volumeReading, 0, 1024, 0, 220);
  //analogWrite(VOL, pwm);
  tone(VOL, getFrequencyFromNote(note), 500);
}

/* Light LEDs 
 *  
 * 
 * Params: int note - the next note to change LED colors based on
 * Returns: None
 */
void lightLEDs(int note) {
  Serial.print("LEDs: note ");
  Serial.print(note);
  Serial.print("\n");
  strip.clear();
  strip.setPixelColor(note, 255, 0, 0);
  /*
  if (note == 1) {
    strip.setPixelColor(1, 255, 0, 0);
    Serial.print("LEDS note: 1\n");
  } else if (note == 2) {
    strip.setPixelColor(2, 255, 0, 0);
    Serial.print("LEDS note: 2\n");
  } else if (note == 3) {
    strip.setPixelColor(3, 255, 0, 0);
    Serial.print("LEDS note: 3\n");
  } else if (note == 4) {
    strip.setPixelColor(4, 255, 0, 0);
    Serial.print("LEDS note: 4\n");
  } else if (note == 5) {
    strip.setPixelColor(5, 255, 0, 0);
    Serial.print("LEDS note: 5\n");
  } else if (note == 6) {
    strip.setPixelColor(6, 255, 0, 0);
    Serial.print("LEDS note: 6\n");
  } else if (note == 7) {
    strip.setPixelColor(7, 255, 0, 0);
    Serial.print("LEDS note: 7\n");
  } else if (note == 8) {
    strip.setPixelColor(8, 255, 0, 0);
    Serial.print("LEDS note: 8\n");
  } else if (note == 9) {
    strip.setPixelColor(9, 255, 0, 0);
    Serial.print("LEDS note: 5\n");
  } else if (note == 10) {
    strip.setPixelColor(10, 255, 0, 0);
    Serial.print("LEDS note: 6\n");
  } else if (note == 11) {
    strip.setPixelColor(11, 255, 0, 0);
    Serial.print("LEDS note: 7\n");
  } else if (note == 12) {
    strip.setPixelColor(12, 255, 0, 0);
    Serial.print("LEDS note: 8\n");
  }
  */
  strip.show();
  return;
}
