
#define INCIDENT_CALIBRATION    330
#define KNOB_MAX_ANALOG_READING 740
#define BUTTON_MEASURE_PIN      A3
#define KNOB_APERTURE_PIN       A0
#define KNOB_ISO_PIN            A1
const double APERATURE_TABLE[]  = {1.0, 1.4, 1.8, 2.0, 2.8, 3.5, 4.0, 4.5, 5.6, 6.3, 8.0, 11.0, 12.7, 16.0, 22.0, 32.0};
const int ISO_TABLE[]           = {6, 12, 25, 50, 100, 160, 200, 400, 800, 1600, 3200};


/* ------------------------------ */

#include <math.h>
#include <Wire.h>
#include <BH1750.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
BH1750 bh1750;

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
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();

  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);

  display.println("READY");
  display.println("press");
  display.println("button");

  display.display();
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
  int apin = analogRead(KNOB_APERTURE_PIN);
  //Serial.print("apin=");
  //Serial.println(apin);
  int ipin = analogRead(KNOB_ISO_PIN);
  //Serial.print("ipin=");
  //Serial.println(ipin);
  apertureIndex = round(map(apin, 0, KNOB_MAX_ANALOG_READING, 0, sizeof(APERATURE_TABLE) / sizeof(float)));
  isoIndex = round(map(ipin, 0, KNOB_MAX_ANALOG_READING, 0, sizeof(ISO_TABLE) / sizeof(int)));
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

  // output result to OLED
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
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
    display.print("EV: ");
    display.println(EV, 1);
    display.print("ISO ");
    display.println(iso);
    display.print("-- f/");
    display.println(aperature, 1);
    display.print("-- ");
    if (shutterspeed > 1) {
      display.print("1/");
      display.print(shutterspeed, 0);
    } else {
      display.print((1 / shutterspeed), 0);
    }
    display.println("s");

  } else {
    Serial.println("Exposure out of bounds");
    display.println("Exposure");
    display.println("value");
    display.println("out of");
    display.println("bounds");
  }
  display.display();
}
