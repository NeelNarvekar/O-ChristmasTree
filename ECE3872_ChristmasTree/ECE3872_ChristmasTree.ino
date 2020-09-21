#define SPDT_PLAYMODE 4
#define INTERRUPT_1   2
#define INTERRUPT_2   3

#define LIVE          1
#define RECORDING     0

volatile int state_var_0;
volatile int state_var_1;

// NOTE! Arduino Nano only has 2,3 as interrupt pins...

void setup() {
  updateStateVar0();
  updateStateVar1();
  // put your setup code here, to run once:
  pinMode(ROTARY_RECORD, INPUT);
  pinMode(ROTARY_STOP, INPUT);
  pinMode(ROTARY_PLAY, INPUT);
  pinMode(SPDT_PLAYMODE, INPUT);
  pinMode(BTN_RESET, INPUT);
  attachInterrupt(digitalPinToInterrupt(INTERRUPT_1), updateStateVar0(), CHANGE);
  attachInterrupt(digitalPinToInterrupt(INTERRUPT_2), updateStateVar1(), CHANGE);
}

void loop() {
  // put your main code here, to run repeatedly:
  stateMachineHandler();
}

void updateStateVar0() {
  state_var_0 = digitalRead(INTERRUPT_1);
}

void pdateStateVar1() {
  state_var_1 = digitalRead(INTERRUPT_2);
}

void stateMachineHandler() {
  if (state_var_0 == 0) {
    if (state_var_1 == 0) {
      reset(); // 0 0
    } else {
      rotaryPlay(); // 0 1
    }
  } else {
    if (state_var_1 == 0) {
      rotaryRecord(); // 1 0
    } else {
      rotaryStop(); // 1 1
    }
  }
}

//STATE FUNCTIONS

void rotaryRecord() {
  while (state_var_0 == 1 && state_var_1 == 0) {
    recordAudioData();
  }
  stateMachineHandler();
}

void rotaryStop() {
  while (state_var_0 == 1 && state_var_1 == 1) {
    
  }
  stateMachineHandler();
}

void rotaryPlay() {
  while (state_var_0 == 0 && state_var_1 == 1) {
    int mode = digitalRead(SPDT_PLAYMODE);
    play();
  }
  stateMachineHandler();
}

void reset() {
  unsigned long time_start = millis();
  while (state_var_0 == 0 && state_var_1 == 0) {
    if (digitalRead(BTN_RESET) == HIGH && (millis() - time_start >= 3000)) {
      eraseRecording();
    }
  }
  stateMachineHandler();
}

// HELPER FUNCTIONS

void eraseRecording() {
  
}

void getFrequency() {
  // connor
}

void play() {
  if (mode == LIVE) {
    moveMotors(); 
    lightLEDs();
    speaker(); // neel
  } else if (mode == RECORDING) {
    moveMotors(); 
    lightLEDs();
    speaker(); // neel
  } 
}

void recordAudioData() {
  getFrequency();
  // connor
}

void moveMotors() { //tony
  
}

void speaker(note) {
  // play note?
}

void lightLEDs() { //tony
  
}


