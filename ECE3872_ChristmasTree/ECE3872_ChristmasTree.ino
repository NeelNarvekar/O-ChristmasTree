#include <Adafruit_NeoPixel.h>
#include <Servo.h>

// Digital Pin Definitions
#define ECHO      0
#define TRIG      1
#define INTERRUPT_1   2
#define INTERRUPT_2   3
#define SPDT_PLAYMODE 4
#define RESET_LED     5
#define SERVO_1_3     6
#define STATE_LED     7
#define VOL           9
#define SERVO_2_4     10
#define STOP          12
#define LED           13

// Analog Pin Definitions
#define POT           A0

// Values
#define LIVE          1
#define RECORDING     0
#define NOTES_STORED 100
#define NUM_LEDS 20

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
  // put your setup code here, to run once:
  myservo_1_3.attach(6);
  myservo_2_4.attach(10);
  strip.begin();
  strip.show();
  Serial.begin(9600);
  pinMode(STOP, INPUT);
  pinMode(STATE_LED, OUTPUT);
  
  eraseRecording();
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
  
  if (state_var_0 == 0) {
    if (state_var_1 == 0) {
      reset(); // 0 0
      //Serial.print("0 0\n");
    } else {
      rotaryPlay(); // 0 1
      //Serial.print("0 1\n");
    }
  } else {
    if (state_var_1 == 0) {
      rotaryRecord(); // 1 0
        //Serial.print("1 0\n");
    } else {
      if (digitalRead(STOP) == 0) {
        rotaryStop(); // 1 1
        //Serial.print("1 1\n");
      }
    }
  }
}

//STATE FUNCTIONS

void rotaryRecord() {
  // if out of space then hold solid LED
  while (state_var_0 == 1 && state_var_1 == 0) { // enter while loop if room to record
    //Serial.print("RECORD STATE\n");
    digitalWrite(STATE_LED, HIGH);
    recordAudioData();
    delay(240); // these will mess up recording of notes. Once recordAudioData() is populated, should take enough time to replicate blink effect
    digitalWrite(STATE_LED, LOW);
    delay(240);
  }
  is_recorded_data = true;
  stateMachineHandler();
}

void rotaryStop() {
  //Serial.print("STOP STATE\n");
  while (state_var_0 == 1 && state_var_1 == 1 && digitalRead(STOP) == 0) {
     // Should we do anything in stop?
  }
  stateMachineHandler();
}

void rotaryPlay() {
  while (state_var_0 == 0 && state_var_1 == 1) {
    //Serial.print("PLAY STATE\n");
    digitalWrite(STATE_LED, HIGH);
    int mode = digitalRead(SPDT_PLAYMODE);
    play(mode);
  }
  digitalWrite(STATE_LED, LOW);
  stateMachineHandler();
}

void reset() {
  unsigned long time_start = millis();
  while (state_var_0 == 0 && state_var_1 == 0) {
    //Serial.print("RESET STATE\n");
    digitalWrite(RESET_LED, HIGH);
    if (millis() - time_start >= 3000) {
      //Serial.print("ERASING RECORDING\n");
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
 * Connor
 * 
 * Clears the recorded notes array and resets the record/play indices
 * 
 * Params: None
 * Returns: None
 */
void eraseRecording() {
  // Connor
  for (int i = 0; i < NOTES_STORED; ++i) {
    recorded_notes[i] = 0;
  }
  idx_record = 0;
  idx_play = NOTES_STORED;
  is_recorded_data = false;
}

/* Get Frequency
 *  Connor
 * 
 * Reads audio input device (mic/aux/pushbuttons) to determine frequency of desired note
 * 
 * SIMULATION: Gets random frequency between 108-210 Hz
 * 
 * Params: None
 * Returns: int frequency - the raw frequency value TOD
 */
float getFrequency() {
  // SIMULATION BEHAVIOR: RANDOM NUMBER
  return random(108.0,210);
}


/* Get Note From Freqency
 *  Connor
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
    else /*if (freq < 119.956)*/ return 12;
//  return -1; 
}

/* Get Frequency from Note
 *  Connor
 *  
 * Provides the frequency of a note's mapping
 *  
 * Params: int note - encoded note data
 * Returns: frequency corresponding to that note
 */
inline float getFrequencyFromNote(int note) {
  return freqs[note-1];
}


/* Get Next Recorded Note
 *  Connor
 *  
 * Gives the next note in the recording (if applicable) and increments the playback index
 * 
 * Params: None
 * Returns: Integer encoding of the next recorded note for playback
 */
int getNextRecordedNote() {
  if (idx_play < idx_record && idx_play < NOTES_STORED)
    return recorded_notes[idx_play++ % idx_record];
  else 
    return -1; // Error code - no more data
}


/* Set Next Recorded Note
 *  Connor
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
    return -1; // Error code
}


void play(int mode) {
  int note;
  if (mode == LIVE) {
    // Live
    note = getNoteFromFrequency(getFrequency());
  } else {
    // Recording
    note = getNextRecordedNote();
  } 
  moveMotors(note); 
  delay(500);
  lightLEDs(note);
  delay(500);
  speaker(note);
  delay(500);
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
  float freq = getFrequency();
  int note = getNoteFromFrequency(freq);
  Serial.print(note);
  Serial.print("\n");
  if (setNextRecordedNote(note) == -1) {
     //TODO: what to do if you're out of space?
  }
}

void moveMotors(int note) {
  //Serial.print("MADE IT TO MOVE\n");
  if (note == 1) {
    myservo_1_3.write(90);
    myservo_2_4.write(90);
    //Serial.print("Move note: 1\n");
  } else if (note == 2) {
    myservo_1_3.write(97.5);
    myservo_2_4.write(82.5);
    //Serial.print("Move note: 2\n");
  } else if (note == 3) {
    myservo_1_3.write(105);
    myservo_2_4.write(75);
    //Serial.print("Move note: 3\n");
  } else if (note == 4) {
    myservo_1_3.write(112.5);
    myservo_2_4.write(67.5);
    //Serial.print("Move note: 4\n");
  } else if (note == 5) {
    myservo_1_3.write(120);
    myservo_2_4.write(60);
    //Serial.print("Move note: 5\n");
  } else if (note == 6) {
    myservo_1_3.write(127.5);
    myservo_2_4.write(52.5);
    //Serial.print("Move note: 6\n");
  } else if (note == 7) {
    myservo_1_3.write(135);
    myservo_2_4.write(45);
    //Serial.print("Move note: 7\n");
  } else if (note == 8) {
    myservo_1_3.write(142.5);
    myservo_2_4.write(37.5);
    //Serial.print("Move note: 8\n");
  } else if (note == 9) {
    myservo_1_3.write(150);
    myservo_2_4.write(30);
    //Serial.print("Move note: 9\n");
  } else if (note == 10) {
    myservo_1_3.write(157.5);
    myservo_2_4.write(22.5);
    //Serial.print("Move note: 10\n");
  } else if (note == 11) {
    myservo_1_3.write(165);
    myservo_2_4.write(15);
    //Serial.print("Move note: 11\n");
  } else if (note == 12) {
    myservo_1_3.write(172.5);
    myservo_2_4.write(7.5);
    //Serial.print("Move note: 12\n");
  }
  return;
}

void speaker(int note) {
  //int volumeReading = analogRead(POT);
  //byte pwm = map(volumeReading, 0, 1024, 0, 220);
  //analogWrite(VOL, pwm);
  tone(VOL, getFrequencyFromNote(note), 500);
}

void lightLEDs(int note) { //tony
  //Serial.print("MADE IT TO LIGHTLEDS\n");
  if (note == 1) {
    strip.clear();
    strip.setPixelColor(1, 255, 0, 0);
    //Serial.print("LEDS note: 1\n");
  } else if (note == 2) {
    strip.clear();
    strip.setPixelColor(2, 255, 0, 0);
    //Serial.print("LEDS note: 2\n");
  } else if (note == 3) {
    strip.clear();
    strip.setPixelColor(3, 255, 0, 0);
    //Serial.print("LEDS note: 3\n");
  } else if (note == 4) {
    strip.clear();
    strip.setPixelColor(4, 255, 0, 0);
    //Serial.print("LEDS note: 4\n");
  } else if (note == 5) {
    strip.clear();
    strip.setPixelColor(5, 255, 0, 0);
    //Serial.print("LEDS note: 5\n");
  } else if (note == 6) {
    strip.clear();
    strip.setPixelColor(6, 255, 0, 0);
    //Serial.print("LEDS note: 6\n");
  } else if (note == 7) {
    strip.clear();
    strip.setPixelColor(7, 255, 0, 0);
    //Serial.print("LEDS note: 7\n");
  } else if (note == 8) {
    strip.clear();
    strip.setPixelColor(8, 255, 0, 0);
    //Serial.print("LEDS note: 8\n");
  } else if (note == 9) {
    strip.clear();
    strip.setPixelColor(9, 255, 0, 0);
    //Serial.print("LEDS note: 9\n");
  } else if (note == 10) {
    strip.clear();
    strip.setPixelColor(10, 255, 0, 0);
    //Serial.print("LEDS note: 10\n");
  } else if (note == 11) {
    strip.clear();
    strip.setPixelColor(11, 255, 0, 0);
    //Serial.print("LEDS note: 11\n");
  } else if (note == 12) {
    strip.clear();
    strip.setPixelColor(12, 255, 0, 0);
    //Serial.print("LEDS note: 12\n");
  }
  strip.show();
  return;
}