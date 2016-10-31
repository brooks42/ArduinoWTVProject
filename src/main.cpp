// helpful links:
// http://nerdclub-uk.blogspot.com/2016/06/playing-audio-with-wtv020m01-and-arduino.html

#include <Arduino.h>
#include <SPI.h>

// you can pass these to play sounds as well
#define DOORBELL 0x0000
#define CREEKY_DOOR 0x0001
#define SCARY_LAUGH 0x0002
#define THUNDER 0x0003
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

int buttonStates[5];

#define SPI_SLAVE_PIN 6 // set low to write to SPI slave devices
#define SPI_TRANSFER_PIN 7 // data I/O pin

#define SLAVE_IS_PLAYING_PIN 8 // a pin set high while the "MP3 player" is playing

// we'll assume we're not playing on boot
bool playing = false;
bool playingDoorbell = false; // set when the doorbell sound should be playing

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
int readButtonStates () {

  int buttonSet = -1;

  // read all of the button states...
  buttonStates[0] = digitalRead(BUTTON_PIN_1);
  buttonStates[1] = digitalRead(BUTTON_PIN_2);
  buttonStates[2] = digitalRead(BUTTON_PIN_3);
  buttonStates[3] = digitalRead(BUTTON_PIN_4);
  buttonStates[4] = digitalRead(BUTTON_PIN_5);

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
  playing = true; // just in case
  /*
  With most SPI devices, after SPI.beginTransaction(), you will write the
  slave select pin LOW, call SPI.transfer() any number of times to transfer
  data, then write the SS pin HIGH, and finally call SPI.endTransaction().
  */
  SPI.beginTransaction(SPISettings(14000000, MSBFIRST, SPI_MODE0));
  SPI.transfer16(message); // send the 16-bit message
  SPI.endTransaction();
  playing = false;
}

//
void loop () {
  // loooooooping
  int buttonDown = readButtonStates();

  // TODO: the doorbell sound effect is a special case and should be a
  // press-and-hold deal, so we'll need a state variable for that button
  //  specifically
  switch (buttonDown) {
    case DOORBELL:
      // special case already
      sendSPIMessage(DOORBELL);
      // special doorbell code
    break;
    case CREEKY_DOOR:
      sendSPIMessage(CREEKY_DOOR);
    break;
    case SCARY_LAUGH:
      sendSPIMessage(SCARY_LAUGH);
    break;
    case THUNDER:
      sendSPIMessage(THUNDER);
    break;
    default:
      // do nothing
    break;
  }
}
