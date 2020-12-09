#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <BH1750.h>

#define OLED_RESET 4
Adafruit_SSD1306 display(128, 64, &Wire, OLED_RESET);

BH1750 lightMeter;
String light;

void setup() {
  Serial.begin(9600);
  // Initialize the I2C bus (BH1750 library doesn't do this automatically)
  Wire.begin();
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  lightMeter.begin();
  Serial.println(F("BH1750 Test begin"));
}

void loop() {
  float lux = lightMeter.readLightLevel();
  Serial.print("Light: ");
  Serial.print(lux);
  Serial.println(" lx");

  display.clearDisplay();
  // text display tests
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.print(lux);
  display.println(" lx");
  display.display();
  delay(1000);
}
