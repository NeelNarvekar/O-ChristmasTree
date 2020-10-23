#//include <ServoTimer2.h>

// TESTING FILE, NOT ACTUAL PROJECT CODE!!
#include <Adafruit_MCP4725.h>
#include <Servo.h>
//#include <Tone.h>
//#include <Adafruit_NeoPixel.h>

// Digital Pin Definitions
#define SONAR_ECHO    0
#define SONAR_TRIG    1
#define INTERRUPT_1   2 // RESET * PLAY
#define INTERRUPT_2   3 // RESET * RECORD
#define SPDT_PLAYMODE 4
#define RESET_LED     5
#define SERVO_1_3     6
#define STATE_LED     7
#define SERVO_2_4     10
#define SPEAKER       11
#define STOP          12
#define LED           13


//#define I2C_SDA       18 // DAC
//#define I2C_SCL       19 // DAC

// Analog Pin Definitions
#define POT           A0


// LED Definitions for Blinking Demo!
#define RED           7
#define YELLOW        6
#define GREEN         5 

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

// Servo 
Servo servo;
//Tone speaker;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  
  // Servo Testing
  servo.attach(SERVO_1_3); // Pin 6 servo //*/
  
  // Tone testing
//  speaker.begin(SPEAKER);
  
  // Sonar Testing
  pinMode(SONAR_TRIG, OUTPUT);
  pinMode(SONAR_ECHO, INPUT);
  digitalWrite(SONAR_TRIG, LOW);
  Serial.begin(115200); //*/
}

unsigned long cm = 10;
unsigned long tStart;
unsigned long tEnd;
float freq;
int noteIdx = 0;

int pos = 0;


void loop() {
//  read_sonar_cm();
 // delay(500);
 
  tStart = millis();
  // Get Sonar distance, convert to frequency, play note, move servo
  
  cm = read_sonar_cm();
//  freq = 107.0 * (((float) cm) / 20.0) + 108.0;
  noteIdx = (cm - 2)/3;// getNoteFromFrequency(freq); // 2-35 -> 0 - 11

  if (noteIdx > -1 && noteIdx < 12) {
    // Start tone
    tone(SPEAKER, (unsigned long) freqs[noteIdx]);
  
    // Servo
    servo.write(14*noteIdx+5);
  } else noTone(SPEAKER);

  //Serial.print("Dist: ");
  //Serial.print(cm);
  Serial.print("Note index: ");
  Serial.print(noteIdx);
  Serial.print(", Freq: ");
  Serial.println((unsigned long) freqs[noteIdx]);

  while (tStart + 500 > millis()); // Spin on half-second delay
  //*/
}

// Blocking call to measure distance on sonar
unsigned long read_sonar_cm() {
  digitalWrite(SONAR_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(SONAR_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(SONAR_TRIG, LOW);
  unsigned long cm = pulseIn(SONAR_ECHO, HIGH, 300000) / 58;
//  Serial.print(cm);
//  Serial.print(" cm - in ");
  return cm; // Distance in cm
}

void servoTesting() {
  
  digitalWrite(LED_BUILTIN, HIGH);
    for (pos = 0; pos <= 180; pos += 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    servo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }
  digitalWrite(LED_BUILTIN, LOW);
  for (pos = 180; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
    servo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  } //*/
}

void blink() {
  digitalWrite(STATE_LED, LOW);
  digitalWrite(LED_BUILTIN, LOW);
  delay(600);
  digitalWrite(STATE_LED, HIGH);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(300);
}


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

/*
int getNoteFromFrequency_NOTE(float freq) {
    // Separate frequencies into bins with cutoffs as geometric mean of two frequencies
    if (freq < 113.223) return NOTE_A2;
    else if (freq < 119.956) return NOTE_AS2;
    else if (freq < 127.089) return NOTE_B2;
    else if (freq < 134.646) return NOTE_C3;
    else if (freq < 142.652) return NOTE_CS3;
    else if (freq < 151.134) return NOTE_D3;
    else if (freq < 160.122) return NOTE_DS3;
    else if (freq < 169.643) return NOTE_E3;
    else if (freq < 179.731) return NOTE_F3;
    else if (freq < 190.418) return NOTE_FS3;
    else if (freq < 201.741) return NOTE_G3;
    else if (freq < 213.737) return NOTE_GS3;
    // If the frequency is greater than the cutoff, don't play anything.
    return -1; 
} //*/
