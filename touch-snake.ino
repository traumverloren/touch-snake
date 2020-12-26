/*******************************************************************************

 Bare Conductive Touch Snake
 ------------------------------

 touch_snake.ino - touch triggered hugs from a snake

*******************************************************************************/

// compiler error handling
#include "Compiler_Errors.h"

// Servos!
#include <Servo.h>

// touch includes
#include <MPR121.h>
#include <Wire.h>

// MP3 includes
#include <SPI.h>
#include <SdFat.h>
#include <FreeStack.h>
#include <SFEMP3Shield.h>

// touch constants
const uint32_t BAUD_RATE = 115200;
const uint8_t MPR121_ADDR = 0x5C;
const uint8_t MPR121_INT = 4;

// serial monitor behaviour constants
const bool WAIT_FOR_SERIAL = false;

// MPR121 datastream behaviour constants
const bool MPR121_DATASTREAM_ENABLE = false;

// MP3 constants
SFEMP3Shield MP3player;

// SD card instantiation
SdFat sd;

Servo servo1;  // create servo object to control a servo
Servo servo2;  // create servo object to control a servo

void setup() {
  servo1.attach(5);
  servo2.attach(6);
  Serial.begin(BAUD_RATE);
  pinMode(LED_BUILTIN, OUTPUT);

  if (WAIT_FOR_SERIAL) {
    while (!Serial);
  }

  if (!sd.begin(SD_SEL, SPI_HALF_SPEED)) {
    sd.initErrorHalt();
  }

  if (!MPR121.begin(MPR121_ADDR)) {
    Serial.println("error setting up MPR121");
    switch (MPR121.getError()) {
      case NO_ERROR:
        Serial.println("no error");
        break;
      case ADDRESS_UNKNOWN:
        Serial.println("incorrect address");
        break;
      case READBACK_FAIL:
        Serial.println("readback failure");
        break;
      case OVERCURRENT_FLAG:
        Serial.println("overcurrent on REXT pin");
        break;
      case OUT_OF_RANGE:
        Serial.println("electrode out of range");
        break;
      case NOT_INITED:
        Serial.println("not initialised");
        break;
      default:
        Serial.println("unknown error");
        break;
    }
    while (1);
  }

  MPR121.setInterruptPin(MPR121_INT);

  if (MPR121_DATASTREAM_ENABLE) {
    MPR121.restoreSavedThresholds();
  } else {
    MPR121.setTouchThreshold(40);
    MPR121.setReleaseThreshold(20);
  }

  MPR121.setFFI(FFI_10);
  MPR121.setSFI(SFI_10);
  MPR121.setGlobalCDT(CDT_4US);  // reasonable for larger capacitances
  
  digitalWrite(LED_BUILTIN, HIGH);  // switch on user LED while auto calibrating electrodes
  delay(1000);
  MPR121.autoSetElectrodes();  // autoset all electrode settings
  digitalWrite(LED_BUILTIN, LOW);
}

void hug() {
  servo1.write(100);   // rotate very slowly clockwise
  servo2.write(80);    // rotate very slowly counterclockwise
  delay(6000);
  servo1.write(90);  // stop
  servo2.write(90);  // stop
}

void release() {
  servo1.write(80);   // rotate very slowly counterclockwise
  servo2.write(100);    // rotate very slowly clockwise
  delay(6000);
  servo1.write(90);  // stop
  servo2.write(90);  // stop
}

void loop() {
  MPR121.updateAll();

  // only make an action if we have one or fewer pins touched
  // ignore multiple touches
  if (MPR121.getNumTouches() <= 1) {
    if (MPR121.isNewTouch(0)) {
        digitalWrite(LED_BUILTIN, HIGH);
        hug();
    } else {
      if (MPR121.isNewRelease(0)) {
        digitalWrite(LED_BUILTIN, LOW);
        release();
      }
    }
  }
}
