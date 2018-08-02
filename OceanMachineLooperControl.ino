#include <OneButton.h>
#include <MIDI.h>

MIDI_CREATE_DEFAULT_INSTANCE();

const int greenFxLedPin =  9;   // the number of the Green LED pin
const int halfspeedFxLedPin = 8;      // the number of the Reb LED pin
const int reverseFxLedPin = 7 ;    // the number of the Blue LED pin

const int greenLoopLedPin =  6; // the number of the Green LED pin
const int redLoopLedPin = 5;    // the number of the Red LED pin
const int blueLoopLedPin = 4 ;  // the number of the Blue LED pin

const int fxReversePin =  10;  
const int fxStopPin =     11;  

int reverseState = 0;
int halfState = 0;
int recState = 0; // 0: Clear, 1: Recording, 2: Playing, 3: Dubbing, 4: Stopped
int looperMenuState = 0;

int looperVolume = 60;
OneButton fxButton(12, false);
OneButton recButton(2, false);

void setup() {
  pinMode(greenFxLedPin, OUTPUT);
  pinMode(halfspeedFxLedPin, OUTPUT);
  pinMode(reverseFxLedPin, OUTPUT);
  pinMode(greenLoopLedPin, OUTPUT);
  pinMode(redLoopLedPin, OUTPUT);
  pinMode(blueLoopLedPin, OUTPUT);

  pinMode(fxStopPin, INPUT);
  pinMode(fxReversePin, INPUT);

  recButton.attachOnClick(recClick);
  recButton.attachDoubleClick(recDoubleClick);
  recButton.attachLongPressStart(recLongPress);
//  recButton.attachLongPressStop(recLongPress);
  recButton.setPressTicks(500);
  recButton.setClickTicks(170);
  recButton.setDebounceTicks(25);

  fxButton.attachClick(fxClick);
  fxButton.attachLongPressStop(fxLongPress);

  MIDI.begin(1);  // Listen to all incoming messages

  digitalWrite(blueLoopLedPin, HIGH);
  digitalWrite(reverseFxLedPin, HIGH);
  delay(1000);
  digitalWrite(blueLoopLedPin, LOW);
  digitalWrite(reverseFxLedPin, LOW);
}

void loop() {
  // Soft MIDI THRU
  if (MIDI.read()){
    MIDI.send(
      MIDI.getType(),
      MIDI.getData1(),
      MIDI.getData2(),
      MIDI.getChannel()      
    );
  }
  recButton.tick();
  fxButton.tick();
  checkVolume();
}

void checkVolume(){
  int value = analogRead(A7);
  int volume = 60.0 - (value * 61.0 / 1023.0);
  if (volume != looperVolume){
    looperSetVolume(volume);
  }
}

void fxClick(){
  if (digitalRead(fxReversePin) == HIGH){
    if (reverseState == 1){
      looperReverseDisable();
    }
    else{
      looperReverseEnable();
    }
  }
  else if (digitalRead(fxStopPin) == HIGH){
    looperStop();
  }
  else {
    if (halfState == 1){
      looperHalfspeedDisable();
    }
    else{
      looperHalfspeedEnable();
    }
  }
}

void fxLongPress(){
  if (looperMenuState == 1){
    looperExit();
  }
  else {
    looperEnter();
  }
}

void recClick(){
  switch (recState){
    case 0: // Clear
      looperRec();
      break;
    case 2: // Playing
      // Overdub
      looperDub();
      break;
    default: // 1. Recording 3. Overdubbing 4. Stopped
      // Play
      looperPlay();
  }
}

void recDoubleClick(){
  looperStop();
}

void recLongPress(){
  looperClear();
}

void looperEnter(){
  MIDI.sendControlChange(26,127,1);
  looperMenuState = 1;
}

void looperExit(){
  MIDI.sendControlChange(26,0,1);
  looperMenuState = 0;
}

void looperPlay(){
  MIDI.sendControlChange(28,127,1);
  digitalWrite(greenLoopLedPin, HIGH);
  digitalWrite(redLoopLedPin, LOW);
  digitalWrite(blueLoopLedPin, LOW);
  recState = 2;
}

void looperRec(){
  MIDI.sendControlChange(27,127,1);
  digitalWrite(redLoopLedPin, HIGH);
  recState = 1;
}

void looperDub(){
  MIDI.sendControlChange(27,127,1);
  digitalWrite(greenLoopLedPin, LOW);
  digitalWrite(redLoopLedPin, HIGH);
  recState = 3;
}

void looperStop(){
  if(recState!=0){
    MIDI.sendControlChange(29,127,1);
    digitalWrite(greenLoopLedPin, LOW);
    digitalWrite(redLoopLedPin, LOW);
    digitalWrite(blueLoopLedPin, HIGH);
    recState = 4;
  }
}

void looperClear(){
  MIDI.sendControlChange(30,127,1);
  digitalWrite(greenLoopLedPin, LOW);
  digitalWrite(redLoopLedPin, LOW);
  digitalWrite(blueLoopLedPin, LOW);
  recState = 0;
}

void looperHalfspeedEnable(){
  MIDI.sendControlChange(31,127,1);
  digitalWrite(halfspeedFxLedPin, HIGH);
  halfState = 1;
}

void looperHalfspeedDisable(){
  MIDI.sendControlChange(31,0,1);
  digitalWrite(halfspeedFxLedPin, LOW);
  halfState = 0;
}

void looperReverseEnable(){
  MIDI.sendControlChange(32,127,1);
  digitalWrite(reverseFxLedPin, HIGH);
  reverseState = 1;
}

void looperReverseDisable(){
  MIDI.sendControlChange(32,0,1);
  digitalWrite(reverseFxLedPin, LOW);
  reverseState = 0;
}

void looperSetVolume(int position){
  MIDI.sendControlChange(45,position,1);
  looperVolume = position;
}

