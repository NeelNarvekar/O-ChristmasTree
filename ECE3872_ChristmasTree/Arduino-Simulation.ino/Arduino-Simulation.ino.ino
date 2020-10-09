#include <Adafruit_NeoPixel.h>
#include <Servo.h>

// Digital Pin Definitions
#define ECHO          0
#define TRIG          1
#define INTERRUPT_1   2
#define INTERRUPT_2   3
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

// Analog Pin Definitions
#define POT           A0


void setup() {
  // put your setup code here, to run once:
  pinMode(STATE_LED, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(STATE_LED, LOW);
  delay(1000);
  digitalWrite(STATE_LED, HIGH);
  delay(1000);
}
