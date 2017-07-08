/*
  SerialPassthrough sketch

  Read the GPS data and output to serial monitor

  Some boards, like the Arduino 101, the MKR1000, Zero, or the Micro,
  have one hardware serial port attached to Digital pins 0-1, and a
  separate USB serial port attached to the IDE Serial Monitor.
  This means that the "serial passthrough" which is possible with
  the Arduino UNO (commonly used to interact with devices/shields that
  require configuration via serial AT commands) will not work by default.

  This sketch allows you to  emulate the serial passthrough behaviour.
  Any text you type in the IDE Serial monitor will be written
  out to the serial port on Digital pins 0 and 1, and vice-versa.

  On the 101, MKR1000, Zero, and Micro, "Serial" refers to the USB Serial port
  attached to the Serial Monitor, and "Serial1" refers to the hardware
  serial port attached to pins 0 and 1. This sketch will emulate Serial passthrough
  using those two Serial ports on the boards mentioned above,
  but you can change these names to connect any two serial ports on a board
  that has multiple ports.

   Created 23 May 2016
   by Erik Nyquist
*/

#include "HardwareSerial.h"
HardwareSerial Serial2(2);

void setup() {
  Serial.begin(9600);
  Serial2.begin(9600);
}

void loop() {
  if (Serial.available()) {      // If anything comes in Serial (USB),
    Serial2.write(Serial.read());   // read it and send it out Serial1 (pins 0 & 1)
  }

  if (Serial2.available()) {     // If anything comes in Serial1 (pins 0 & 1)
    Serial.write(Serial2.read());   // read it and send it out Serial (USB)
  }
}
