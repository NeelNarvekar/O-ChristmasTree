#include <Adafruit_NeoPixel.h>

#include <Servo.h>


// INCLUDED LIBRARIES
// END OF INCLUDED LIBRARIES


#define SPDT_PLAYMODE 4
#define INTERRUPT_1   2
#define INTERRUPT_2   3
#define SERVO_1_3     6
#define STATE_LED    7
#define RESET_LED     5
#define SERVO_2_4   10
#define LED       13
#define PLAY      12

#define LIVE          1
#define RECORDING     0

#define POT       A0
#define VOL       9

#define NOTES_STORED 100
#define NUM_LEDS 20

#define A2       110.000
#define B2       123.471
#define C3       130.813
#define D3       146.832
#define E3       164.814
#define F3       174.614
#define G3       195.998
#define Gs3      207.652

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
volatile unsigned int idx_play = NOTES_STORED; // Check that this idx is in bounds before playing!

// NOTE! Arduino Nano only has 2,3 as interrupt pins...

void setup() {
  // put your setup code here, to run once:
  myservo_1_3.attach(6);
  myservo_2_4.attach(10);
  strip.begin();
  strip.show();
  Serial.begin(9600);
  pinMode(PLAY, INPUT);
  pinMode(STATE_LED, OUTPUT);
  
  
//  eraseRecording();
  updateStateVar0();
  updateStateVar1();

  pinMode(SPDT_PLAYMODE, INPUT);
  pinMode(INTERRUPT_1, INPUT_PULLUP);
  pinMode(INTERRUPT_2, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(INTERRUPT_1), updateStateVar0, CHANGE);
  attachInterrupt(digitalPinToInterrupt(INTERRUPT_2), updateStateVar1, CHANGE);
}

void loop() {
  // put your main code here, to run repeatedly:
  stateMachineHandler();
}

void updateStateVar0() {
  //Serial.print("INTERRUPT FIRED 1");
  state_var_0 = digitalRead(INTERRUPT_1);
  digitalWrite(STATE_LED, LOW);
  digitalWrite(RESET_LED, LOW);
}

void updateStateVar1() {
  state_var_1 = digitalRead(INTERRUPT_2);
  digitalWrite(STATE_LED, LOW);
  digitalWrite(RESET_LED, LOW);
}

void stateMachineHandler() {
  //moveMotors(1);
  //delay(1000);
  //moveMotors(2);
  //delay(1000);
  //speaker(1);
  //lightLEDs(1);
  
  if (state_var_0 == 0) {
    if (state_var_1 == 0) {
      reset(); // 0 0
      //Serial.print("0 0\n");
    } else {
      if (digitalRead(PLAY) == 1) {
        rotaryPlay(); // 0 1
        //Serial.print("0 1\n");
      }
    }
  } else {
    if (state_var_1 == 0) {
      rotaryRecord(); // 1 0
        //Serial.print("1 0\n");
    } else {
      rotaryStop(); // 1 1
        //Serial.print("1 1\n");
    }
  }
}

//STATE FUNCTIONS

void rotaryRecord() {
  // if out of space then hold solid LED
  while (state_var_0 == 1 && state_var_1 == 0) { // enter while loop if room to record
    Serial.print("RECORD STATE\n");
    digitalWrite(STATE_LED, HIGH);
    recordAudioData();
    delay(300); // these will mess up recording of notes. Once recordAudioData() is populated, should take enough time to replicate blink effect
    digitalWrite(STATE_LED, LOW);
    delay(300);
  }
  stateMachineHandler();
}

void rotaryStop() {
  while (state_var_0 == 1 && state_var_1 == 1) {
    Serial.print("STOP STATE\n");
  }
  stateMachineHandler();
}

void rotaryPlay() {
  while (state_var_0 == 0 && state_var_1 == 1 && digitalRead(PLAY) == 1) {
    digitalWrite(STATE_LED, HIGH);
    Serial.print("PLAY STATE\n");
    int mode = digitalRead(SPDT_PLAYMODE);
    play(mode);
  }
  digitalWrite(STATE_LED, LOW);
  stateMachineHandler();
}

void reset() {
  unsigned long time_start = millis();
  while (state_var_0 == 0 && state_var_1 == 0) {
    digitalWrite(RESET_LED, HIGH);
    Serial.print("RESET STATE\n");
    if (millis() - time_start >= 3000) {
      Serial.print("ERASING RECORDING\n");
      eraseRecording();
    }
    delay(100);
    digitalWrite(RESET_LED, LOW);
    delay(100);
  }
  stateMachineHandler();
}




// ----------- HELPER FUNCTIONS ---------

/* Erase Recording
 * Connor
 * 
 * Clears the recorded notes array and resets the record/play indices
 * 
 * Params: None
 * Returns: None
 */

void eraseRecording() {
  // connor
  //for (int i = 0; i < NOTES_STORED; ++i) {
  //  recorded_notes = 0.0;
  //}
  //idx_record = 0;
  //idx_play = NOTES_STORED;
}

/* Get Frequency
 * Connor
 * 
 * Reads audio input device (mic/aux/pushbuttons) to determine frequency of desired note
 * 
 * Params: None
 * Returns: int frequency - the raw frequency value TOD
 */
//float getFrequency() {
//  // TODO
//  return -1;
//}


/* Get Note From Freqency
 *  Connor
 *  
 *  Maps a given frequency to its nearest output note
 *  
 *  Params: float frequency - raw frequency data
 *  Returns: The integer mapping of the nearest note to the input frequncy
 */
//int getNoteFromFrequency(float freq) {
//  // TODO
//  return -1; 
//}


/* Get Next Recorded Note
 *  Connor
 *  
 * Gives the next note in the recording (if applicable) and increments the playback index
 * 
 * Params: None
 * Returns: Integer encoding of the next recorded note for playback
 */
//int getNextRecordedNote() {
//  if (idx_play < idx_record && idx_play < NOTES_STORED)
//    return recorded_notes[idx_play++];
//  else 
//    return -1; // Error code
//}


/* Set Next Recorded Note
 *  Connor
 *  
 * Writes the next recorded note to the note storage array
 * 
 * Params: int note - the next note to record
 * Returns: int status - 0 if successful, -1 if out of recording space
 */
//int setNextRecordedNote(int note) {
//  if (idx_record < NOTES_STORED) {
//    recorded_notes[idx_record++] = note;
//    return 0;
//  }
//    return -1; // Error code
//}


void play(int mode) {
  int note;
  if (mode == LIVE) {
    // Live
//    note = getNoteFromFrequency(getFrequency());
    note = 1;
  } else {
    // Recording
//    note = getNextRecordedNote();
    note = 2;
  } 
  moveMotors(note); 
  lightLEDs(note);
  speaker(note); // neel
}


/* Record Audio Data
 *  Connor
 *  
 * Calculates current frequency, retrieves the note, and stores it
 * 
 * Params: None
 * Returns: None
 */
void recordAudioData() {
  //float freq = getFrequency();
  //int note = getNoteFromFrequency(freq);
  //if (setNextRecordedNote(note) == -1) {
    // TODO: what to do if you're out of space?
  //}
  // connor
}

void moveMotors(int note) { //tony
  Serial.print("MADE IT TO MOVE\n");
  if (note == 1) {
    myservo_1_3.write(200);
    Serial.print("MADE IT TO WRITE SERVO\n");
    delay(1000);
    myservo_1_3.write(0);
  } else if (note == 2) {
    myservo_2_4.write(200);
    delay(1000);
    myservo_2_4.write(0);
  }
  return;
}

void speaker(int note) {
  //int volumeReading = analogRead(POT);
  //byte pwm = map(volumeReading, 0, 1024, 0, 220);
  //analogWrite(VOL, pwm);
  tone(VOL, A2, 500);
  delay(1000);
  tone(VOL, B2, 500);
  delay(1000);
  tone(VOL, C3, 500);
  delay(1000);
  tone(VOL, D3, 500);
  delay(1000);
  tone(VOL, E3, 500);
  delay(1000);
  tone(VOL, F3, 500);
  delay(1000);
  tone(VOL, G3, 500);
  delay(1000);
  tone(VOL, Gs3, 500);
  delay(1000);

  // play note?
}

void lightLEDs(int note) { //tony
  for (int i = 0; i < NUM_LEDS; i++) {
  strip.setPixelColor(i, 255, 0, 0);
  strip.show();
  delay(100);
  // set pixel to off, delay(1000)
  strip.setPixelColor(i, 0, 0, 0);
  strip.show();
  delay(100);
  }
}
