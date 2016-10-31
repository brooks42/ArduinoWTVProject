// helpful links:
// http://nerdclub-uk.blogspot.com/2016/06/playing-audio-with-wtv020m01-and-arduino.html

// TODO: maybe implement debouncing if we want to/care:
// https://www.arduino.cc/en/Tutorial/Debounce

// the creeky door sound should override any other sound effect or button activity
// the doorbell should kill the other sounds, except for creeky door
// the other buttons, if pressed while another of those sounds are playing, should kill the sound effect

#include <Arduino.h>
#include <SPI.h>

// you can pass these to play sounds as well
#define CREEKY_DOOR 0x0000
#define DOORBELL 0x0001
#define SCARY_LAUGH 0x0002
#define THUNDER 0x0003
#define SOUND_FIVE 0x0004
// add more sounds here...

// convenient commands for reference:
#define STOP 0xFFFF // stop the device
#define PLAY 0xFFFE // play/pause toggle
#define HIGH_VOLUME 0xFFF7 // set volume to 7

// various pins for button input
#define BUTTON_PIN_1 1
#define BUTTON_PIN_2 2
#define BUTTON_PIN_3 3
#define BUTTON_PIN_4 4
#define BUTTON_PIN_5 5

#define SPI_SLAVE_PIN 6 // set low to write to SPI slave devices
#define SPI_TRANSFER_PIN 7 // data I/O pin

#define SLAVE_IS_PLAYING_PIN 8 // a pin set high while the "MP3 player" is playing

// the event structure we use to manage our state
struct Event {
  int currentSound; // the sound we're currently playing in this event

  // together the below 2 arrays let us calculate button deltas for debouncing
  // if one is HIGH and one is LOW, there was a button delta to that state
  int previousButtonStates[5]; // the button states of the previous event
  int buttonStates[5]; // the button states for this event
};

Event lastTick; // last event we did, to help with debouncing

//
void setup () {

  // button pins
  pinMode(BUTTON_PIN_1, INPUT);
  pinMode(BUTTON_PIN_2, INPUT);
  pinMode(BUTTON_PIN_3, INPUT);
  pinMode(BUTTON_PIN_4, INPUT);
  pinMode(BUTTON_PIN_5, INPUT);

  //
  pinMode(SPI_SLAVE_PIN, OUTPUT);
  pinMode(SPI_TRANSFER_PIN, OUTPUT);

  //
  pinMode(SLAVE_IS_PLAYING_PIN, INPUT);
}

// so with this method we check the button states. If any one button is down, we
// return that button's index
// if more than one button is down, or no buttons are down, we return -1
int parseButtonStates (int* buttonStates) {

  int buttonSet = -1;

  for (int i = 0; i < 4; i++) {
    if (buttonSet == -1) {
      buttonSet = i; // set i to the pressed button
    } else {
      return -1; // we've got at least 2 buttons pressed so break
    }
  }

  return buttonSet;
}

// note: might need this to configure for this specific board:
// https://www.arduino.cc/en/Reference/SPI
// http://www.datasheet-pdf.info/entry/WTV020M01
// begin the SPI communications:
void sendSPIMessage(uint16_t message) {
  /*
  With most SPI devices, after SPI.beginTransaction(), you will write the
  slave select pin LOW, call SPI.transfer() any number of times to transfer
  data, then write the SS pin HIGH, and finally call SPI.endTransaction().
  */
  SPI.beginTransaction(SPISettings(14000000, MSBFIRST, SPI_MODE0));
  SPI.transfer16(message); // send the 16-bit message
  SPI.endTransaction();
}

// read the MP3 chip's active pin to see if we're playing a sound
bool isPlayingSound() {
  return digitalRead(SLAVE_IS_PLAYING_PIN);
}

// compile a little event
Event buildEventTick() {

  Event tempEvent;

  // populate the button states...
  tempEvent.buttonStates[0] = digitalRead(BUTTON_PIN_1);
  tempEvent.buttonStates[1] = digitalRead(BUTTON_PIN_2);
  tempEvent.buttonStates[2] = digitalRead(BUTTON_PIN_3);
  tempEvent.buttonStates[3] = digitalRead(BUTTON_PIN_4);
  tempEvent.buttonStates[4] = digitalRead(BUTTON_PIN_5);

  // populate this tick's previous array with the last tick's current one
  for (int i = 0; i < 4; i++) {
    tempEvent.previousButtonStates[i] = lastTick.buttonStates[i];
  }

  // now if we're playing a sound, populate the sound variable
  if (isPlayingSound()) {
    tempEvent.currentSound = lastTick.currentSound;
  } else {
    tempEvent.currentSound = -1; // no sound
  }

  return tempEvent;
}

// returns true if the button was LOW in the previous tick and is now HIGH
bool buttonWasPressed(int button, Event event) {
  if (event.buttonStates[button] == HIGH &&
    event.previousButtonStates[button] == LOW) {
      return true;
    }
    return false;
  }

// plays a sound and manipulates our current state to help track
  void playSound(uint16_t sound, Event tick) {
    sendSPIMessage(STOP);
    sendSPIMessage(sound);
    tick.currentSound = sound;
  }

  //
  void loop () {
    // loooooooping
    Event thisTick = buildEventTick();
    int soundToPlayIfAny = -1;

    // creeky door overrides everything
    if (thisTick.currentSound == CREEKY_DOOR) {
      lastTick = thisTick;
      delay(50); // help with debouncing
      return; // we're stuck going until the sound is finished
    }

    // the other sounds can override eachother
    int buttonDown = -1;
    for (int i = 0; i < 4; i++) {
      if (buttonWasPressed(i, thisTick)) {
        buttonDown = i;
        break; // just use whatever button they hit first, no biggy
      }
    }

    // if the button we've tapped is a lower value (higher priority) than
    // the current sound, replace it (so CREEKY_DOOR will replace DOORBELL, but
    // not the other way around, etc, but the last 3 sounds will all replace
    // eachother)
    if (buttonDown < thisTick.currentSound ||
       (buttonDown > 1 && thisTick.currentSound > 1)) {
      soundToPlayIfAny = buttonDown;
    }

    //
    if (soundToPlayIfAny != -1) {
      playSound(soundToPlayIfAny, thisTick);
    }

    lastTick = thisTick;
    delay(50); // help with debouncing
  }
