
#define INCIDENT_CALIBRATION    330
#define KNOB_MAX_ANALOG_READING 740
#define BUTTON_MEASURE_PIN      PD2
#define KNOB_APERTURE_PIN       A0
#define KNOB_ISO_PIN            A1
const double APERATURE_TABLE[]  = {1.0, 1.4, 1.8, 2.0, 2.8, 3.5, 4.0, 4.5, 5.6, 6.3, 8.0, 11.0, 12.7, 16.0, 22.0, 32.0};
const int ISO_TABLE[]           = {6, 12, 25, 50, 100, 160, 200, 400, 800, 1600, 3200};


/* ------------------------------ */

#include <math.h>
#include <Wire.h>
#include <BH1750.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiAvrI2c.h"

BH1750 bh1750;
SSD1306AsciiAvrI2c oled;

bool started = false;
int EV, apertureIndex, apertureIndexOld, isoIndex, isoIndexOld;
long lux;

void setup() {

  // initialize serial port
  Serial.begin(9600);
  Wire.begin();

  pinMode(BUTTON_MEASURE_PIN, INPUT_PULLUP);

  bh1750.begin(BH1750::ONE_TIME_HIGH_RES_MODE_2);

  // "initialize" BH1750 by using it once
  bh1750.readLightLevel();

  // initialize OLED
  oled.begin(&Adafruit128x64, 0x3C);
  oled.setFont(System5x7);
  oled.set2X();

  oled.clear();
  oled.println("READY");
  oled.println("press");
  oled.println("button");
}

void loop() {

  //read status from button and potentiometers
  getKnobIndex();

  if (buttonPressed()) { //measure light
    if (!started) started = true;
    displayExposureSetting(true);
    delay(150);

  } else { //change aperture/ISO settings
    if ((apertureIndex != apertureIndexOld || isoIndex != isoIndexOld) && started) {
      displayExposureSetting(false);
      delay(150);
    }
  }

  //record potentiometer previous status
  apertureIndexOld = apertureIndex;
  isoIndexOld = isoIndex;

  delay(50);
}

// read if the button is pressed (pulled low)
bool buttonPressed() {
  return !digitalRead(BUTTON_MEASURE_PIN);
}

// map knob analog readings to array indexes
void getKnobIndex() {
  apertureIndex = round(map(analogRead(KNOB_APERTURE_PIN), 0, KNOB_MAX_ANALOG_READING, 0, sizeof(APERATURE_TABLE) / sizeof(float)));
  isoIndex = round(map(analogRead(KNOB_ISO_PIN), 0, KNOB_MAX_ANALOG_READING, 0, sizeof(ISO_TABLE) / sizeof(int)));
}

// measure/calculate and display exposure settings
void displayExposureSetting(bool measureNewEV) {

  double aperature = APERATURE_TABLE[apertureIndex];
  int iso = ISO_TABLE[isoIndex];

  // measure light level (illuminance) and get a new lux value
  if (measureNewEV) {
    lux = bh1750.readLightLevel();
    Serial.print("Measured illuminance = ");
    Serial.print(lux);
    Serial.println(" lux");
  }

  //calculate EV
  EV = log10(lux * iso / INCIDENT_CALIBRATION) / log10(2);

  if (isfinite(EV)) { //calculate shutter speed if EV is neither NaN nor infinity

    // calculate shutter speed
    double shutterspeed = (pow(2, EV) / pow(aperature, 2));

    // output result to serial port
    Serial.print("Exposure settings: EV = ");
    Serial.print(EV);
    Serial.print(", ISO = ");
    Serial.print(iso);
    Serial.print(", aperture = f/");
    Serial.print(aperature, 1);
    Serial.print(", ");
    Serial.print("shutter speed = ");
    if (shutterspeed > 1) {
      Serial.print("1/");
      Serial.print(shutterspeed);
    } else {
      Serial.print((1 / shutterspeed));
    }
    Serial.println("s");

    // output result to OLED
    oled.clear();
    oled.print("EV: ");
    oled.println(EV, 1);
    oled.print("ISO ");
    oled.println(iso);
    oled.print("-- f/");
    oled.println(aperature, 1);
    oled.print("-- ");
    if (shutterspeed > 1) {
      oled.print("1/");
      oled.print(shutterspeed, 0);
    } else {
      oled.print((1 / shutterspeed), 0);
    }
    oled.println("s");

  } else {
    Serial.println("Exposure out of bounds");
    oled.clear();
    oled.println("Exposure");
    oled.println("value");
    oled.println("out of");
    oled.println("bounds");
  }
}
