// helpful links:
// http://nerdclub-uk.blogspot.com/2016/06/playing-audio-with-wtv020m01-and-arduino.html

#include <Arduino.h>
#include <SPI.h>

// you can pass these to play sounds as well
#define CREEKY_DOOR x0001
#define DOORBELL x0002
#define SCARY_LAUGH x0003
#define THUNDER x0004
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

#define SPI_SLAVE_PIN 5 // set low to write to SPI slave devices
#define SPI_TRANSFER_PIN 6 // data I/O pin

// we'll assume we're not playing on boot
bool playing = false;

//
void setup () {
  pinMode(BUTTON_PIN_1, INPUT);
  pinMode(BUTTON_PIN_2, INPUT);
  pinMode(BUTTON_PIN_3, INPUT);
  pinMode(BUTTON_PIN_4, INPUT);
}

//
void loop () {

}

// note: might need this to configure for this specific board:
// https://www.arduino.cc/en/Reference/SPI
// http://www.datasheet-pdf.info/entry/WTV020M01
// begin the SPI communications:
void sendSPIMessage() {
  playing = true; // just in case
  /*
  With most SPI devices, after SPI.beginTransaction(), you will write the
  slave select pin LOW, call SPI.transfer() any number of times to transfer
  data, then write the SS pin HIGH, and finally call SPI.endTransaction().*/
  SPI.beginTransaction(SPISettings(14000000, MSBFIRST, SPI_MODE0));

  SPI.endTransaction();
  playing = false;
}
