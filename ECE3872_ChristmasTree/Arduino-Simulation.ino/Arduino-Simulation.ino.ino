// TESTING FILE, NOT ACTUAL PROJECT CODE!!
#include <Adafruit_MCP4725.h>
#include <Servo.h>
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
 // space for 8!
#define VOL           9 // this will eventually be analog, connected to speaker
#define SERVO_2_4     10
 // space for 11!
#define STOP          12
#define LED           13


//#define I2C_SDA       18 // DAC
//#define I2C_SCL       19 // DAC

// Analog Pin Definitions
#define POT           A0


// LED Definitions for Demo!
#define RED           7
#define YELLOW        6
#define GREEN         5 

// Servo 
Servo servo;


void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  
  // Servo Testing
  servo.attach(SERVO_1_3); //*/
  
/*
  // Sonar Testing
  pinMode(SONAR_TRIG, OUTPUT);
  pinMode(SONAR_ECHO, INPUT);
  digitalWrite(SONAR_TRIG, LOW);
  Serial.begin(115200); //*/
}

unsigned long cm;
unsigned long tStart;
unsigned long tEnd;

int pos = 0;

void loop() {
  // Servo testing
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

/*
  // Sonar testing 
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(RED, LOW);
  digitalWrite(YELLOW, LOW);
  digitalWrite(GREEN, LOW);
  delay(10);
  tStart = millis();
  cm = read_sonar_cm();
  tEnd = millis();
  Serial.print(tEnd - tStart);
  Serial.println(" ms elapsed");
  digitalWrite(LED_BUILTIN, LOW);
  if (cm < 12) digitalWrite(RED, HIGH);
  else { 
    if (cm < 24) digitalWrite(YELLOW, HIGH); 
    else digitalWrite(GREEN, HIGH); 
  }
  if (cm > 24) digitalWrite(GREEN, HIGH);
  delay(400); //*/
}

// Blocking call to measure distance on sonar
unsigned long read_sonar_cm() {
  digitalWrite(SONAR_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(SONAR_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(SONAR_TRIG, LOW);
  unsigned long cm = pulseIn(SONAR_ECHO, HIGH, 300000) / 58;
  Serial.print(cm);
  Serial.print(" cm - ");
  return cm; // Distance in cm
}


void blink() {
  digitalWrite(STATE_LED, LOW);
  digitalWrite(LED_BUILTIN, LOW);
  delay(600);
  digitalWrite(STATE_LED, HIGH);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(300);
}
