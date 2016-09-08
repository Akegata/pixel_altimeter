// LED Skydive Altimeter by Martin Hovmöller
// To do:
// 1) Detect descent, don't light up LEDs on ascent.
// 2) Deep sleep power mode.

#include <EEPROM.h>
#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_BMP085.h>

Adafruit_BMP085 bmp;

#define PIN 2
int num_leds = 2; // Specified how many LEDs are in the array.
Adafruit_NeoPixel strip = Adafruit_NeoPixel(num_leds, PIN);

// Define different colors for easier use.
uint32_t blue      = strip.Color(0, 0, 255);
uint32_t cyan      = strip.Color(36, 182, 255);
uint32_t cyan_dim  = strip.Color(0, 20, 20);
uint32_t green     = strip.Color(0, 255, 0);
uint32_t red       = strip.Color(255, 0, 0);
uint32_t violet    = strip.Color(109, 36, 255);
uint32_t white_dim = strip.Color(20, 20, 20);
uint32_t yellow    = strip.Color(255, 255, 0);
uint32_t off       = strip.Color(0, 0, 0);

int agl                 = 0;
int baseline_address    = 1; // Address in the EEPROM where the baseline reading should be stored.
int powercycles_address = 0;
int startup             = 0;
int powercycles;
double baseline;

struct MyObject {
  double field1;
  byte field2;
  char name[10];
};
MyObject read_baseline;

// Sets the specified numbers of LEDs to the specifiedcolor.
int setLEDColors(int nr_leds, uint32_t color) {
  for (int i = 0; i < nr_leds; i++) {
    strip.setPixelColor(i, off);
  }
  strip.show();
}

// Cycles through the LED's, only lighting up one LED at a time.
// If we're only using one LED, this can be scrapped for minimal code size.
int cycleLEDColors(int nr_leds, uint32_t color, int cycle_time) {
  setLEDColors(num_leds, off);
  for (uint16_t i = 0; i < nr_leds; i++) {
    strip.setPixelColor(i, color);
    strip.show();
    delay(cycle_time);
    strip.setPixelColor(i, off);
    strip.show();
  }
  strip.show();
}

// Blinks the specified number of LED's at the specified interval, with the specified color.
int blinkLEDColors(int nr_leds, uint32_t color, int on_time, int off_time) {
  for (uint16_t i = 0; i < nr_leds; i++) {
    strip.setPixelColor(i, color);
  }
  strip.show();
  delay(on_time);

  for (uint16_t i = 0; i < nr_leds; i++) {
    strip.setPixelColor(i, off);
  }
  strip.show();
  delay(off_time);
}

void setup() {
 
  strip.begin();
  strip.show();

  powercycles = (EEPROM.read(powercycles_address));

  // On the third power cycle, reset the calibration
  if (powercycles == 2) {
    baseline = bmp.readAltitude();
    EEPROM.put(baseline_address, baseline);

    blinkLEDColors(num_leds, red, 500, 500);
  }
  // Update the powercycle count.
  else {
    powercycles++;
    EEPROM.write(powercycles_address, powercycles);
    cycleLEDColors(num_leds, blue, 200);
    delay(2000);
  }

  // Reset the power cycle count and read the baseline from EEPROM.
  powercycles = (0);
  EEPROM.write(0, powercycles);
  EEPROM.get(baseline_address, read_baseline);

  // Blink LED's green to indicate that the altimeter is running.
  blinkLEDColors(num_leds, green, 100, 100);
}

void loop() {
  agl = bmp.readAltitude() - read_baseline.field1;

  // Light up or blink the LEDs in different colors depending on altitude.
  if (agl > 3500) {
    setLEDColors(num_leds, blue);
  }
  else if (agl < 3500 && agl >= 3000) {
    blinkLEDColors(num_leds, blue, 800, 800);
  }
  else if (agl < 3000 && agl >= 2500) {
    setLEDColors(num_leds, green);
  }
  else if (agl < 2500 && agl >= 2000) {
    blinkLEDColors(num_leds, green, 800, 800);
  }
  else if (agl < 2000 && agl >= 1500) {
    setLEDColors(num_leds, yellow);
  }
  else if (agl < 1500 && agl >= 1000) {
    setLEDColors(num_leds, red);
  }
  else if (agl < 1000 && agl >= 500) {
    blinkLEDColors(num_leds, red, 300, 300);
  }
}
