#define SPDT_PLAYMODE 4
#define INTERRUPT_1   2
#define INTERRUPT_2   3

#define LIVE          1
#define RECORDING     0

#define NOTES_STORED 100
// Global variables

volatile int state_var_0;
volatile int state_var_1;

// Recording and playing back different notes
volatile int recorded_notes[NOTES_STORED];
volatile unsigned int idx_record = 0;
volatile unsigned int idx_play = NOTES_STORED; // Check that this idx is in bounds before playing!

// NOTE! Arduino Nano only has 2,3 as interrupt pins...

void setup() {
  // put your setup code here, to run once:
  eraseRecording();
  updateStateVar0();
  updateStateVar1();
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
    play(mode);
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
  for (int i = 0; i < NOTES_STORED; ++i) {
    recorded_notes = 0.0;
  }
  idx_record = 0;
  idx_play = NOTES_STORED;
}

/* Get Frequency
 * Connor
 * 
 * Reads audio input device (mic/aux/pushbuttons) to determine frequency of desired note
 * 
 * Params: None
 * Returns: int frequency - the raw frequency value TOD
 */
float getFrequency() {
  // TODO
  return -1;
}


/* Get Note From Freqency
 *  Connor
 *  
 *  Maps a given frequency to its nearest output note
 *  
 *  Params: float frequency - raw frequency data
 *  Returns: The integer mapping of the nearest note to the input frequncy
 */
int getNoteFromFrequency(float freq) {
  // TODO
  return -1; 
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
    return recorded_notes[idx_play++];
  else 
    return -1; // Error code
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
  float freq = getFrequency();
  int note = getNoteFromFrequency(freq);
  if (setNextRecordedNote(note) == -1) {
    // TODO: what to do if you're out of space?
  }
  // connor
}

void moveMotors(int note) { //tony
  
}

void speaker(int note) {
  // play note?
}

void lightLEDs(int note) { //tony
  
}
