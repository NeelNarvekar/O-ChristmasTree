#define ROTARY_RECORD 11
#define ROTARY_STOP 12
#define ROTARY_PLAY 13
#define SPDT_PLAYMODE 4
#define BTN_RESET 5

#define LIVE 1
#define RECORDING 0

// NOTE! Arduino Nano only has 2,3 as interrupt pins...


void setup() {
  // put your setup code here, to run once:
  pinMode(ROTARY_RECORD, INPUT);
  pinMode(ROTARY_STOP, INPUT);
  pinMode(ROTARY_PLAY, INPUT);
  pinMode(SPDT_PLAYMODE, INPUT);
  pinMode(BTN_RESET, INPUT);
  attachInterrupt(digitalPinToInterrupt(ROTARY_RECORD), stateMachineHandler(), RISING);
  attachInterrupt(digitalPinToInterrupt(ROTARY_STOP), stateMachineHandler(), RISING);
  attachInterrupt(digitalPinToInterrupt(ROTARY_PLAY), stateMachineHandler(), RISING);
  attachInterrupt(digitalPinToInterrupt(BTN_RESET), resetInterruptHandler(), RISING);
}

void loop() {
  // put your main code here, to run repeatedly:
  stateMachineHandler();
}

void stateMachineHandler() {
  if (digitalRead(ROTARY_RECORD) == HIGH) {
    rotaryRecord();
  } else if (digitalRead(ROTARY_STOP) == HIGH) {
    rotaryStop();
  } else if (digitalRead(ROTARY_PLAY) == HIGH) {
    rotaryPlay();
  }
}

void rotaryRecord() {
  while (digitalRead(ROTARY_RECORD) == HIGH) {
    recordAudioData();
  }
}

void rotaryStop() {
  unsigned long time_start = millis();
  while (digitalRead(ROTARY_STOP) == HIGH || digitalRead(BTN_RESET) == HIGH) {
    if (digitalRead(BTN_RESET) == HIGH && (millis() - time_start >= 3000)) {
      eraseRecording();
    }
  }
}

void rotaryPlay() {
  while (digitalRead(ROTARY_PLAY) == HIGH) {
    int mode = digitalRead(SPDT_PLAYMODE);
    if (mode == LIVE) {
      playLive();
    } else if (mode == RECORDING) {
      playRecording();
    }
  }
}

void resetInterruptHandler() {
  rotaryStop();
}

void eraseRecording() {
  
}

void playLive() {
  moveMotors();
  lightLEDs();

}

void playRecording() {

}

void recordAudioData() {
  
}

void moveMotors() {
  
}

void lightLEDs() {
  
}


