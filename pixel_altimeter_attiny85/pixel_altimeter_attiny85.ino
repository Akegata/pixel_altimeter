// NeoPixel Ring simple sketch (c) 2013 Shae Erisson
// Released under the GPLv3 license to match the rest of the
// Adafruit NeoPixel library

#include <Adafruit_NeoPixel.h>
#include <avr/power.h> 
#include <TinyWireM.h>
#include <tinyBMP085.h>
#include <EEPROM.h>


#define PIXELPIN 4

//#define simulation

#ifdef simulation
uint16_t  agl = 4000;
#else
uint16_t agl = 10;
#endif

Adafruit_NeoPixel pixels(1, PIXELPIN, NEO_GRB + NEO_KHZ800);
tinyBMP085 bmp;

// Define different colors for easier use.
uint32_t blue      = pixels.Color(0, 0, 255);
uint32_t green     = pixels.Color(0, 255, 0);
uint32_t red       = pixels.Color(255, 0, 0);
uint32_t yellow    = pixels.Color(255, 255, 0);
uint32_t off       = pixels.Color(0, 0, 0);

uint16_t baseline_address    = 1; // Address in the EEPROM where the baseline reading should be stored.
uint16_t powercycles_address = 0;
int startup             = 0;
int powercycles;
double baseline;

struct eeprom_entry {
  double field1;
  byte field2;
  char name[10];
};
eeprom_entry read_baseline;

// Sets the specified numbers of LEDs to the specifiedcolor.
int setLEDcolor(uint32_t color) {
  pixels.setPixelColor(0, color);
  pixels.show();
}

// Blinks the specified number of LED's at the specified interval, with the specified color.
int blinkLEDcolor(uint32_t color, int on_time, int off_time) {
  pixels.setPixelColor(0, color);
  pixels.show();
  delay(on_time);
  pixels.setPixelColor(0, off);
  pixels.show();
  delay(off_time);
}

void setup() {
  bmp.begin();
  pixels.setPixelColor(0, off);
  pixels.begin();
  pixels.show();

  powercycles = (EEPROM.read(powercycles_address));

  // On the third power cycle, reset the calibration
  if (powercycles == 2) {
    int baseline;
    baseline = bmp.readAltitudeSTDdm();
    EEPROM.put(baseline_address, baseline);

    blinkLEDcolor(red, 500, 500);
  }
  // Update the powercycle count.
  else {
    powercycles++;
    EEPROM.write(powercycles_address, powercycles);
    blinkLEDcolor(blue, 200, 200);
    delay(1000);
  }

  // Reset the power cycle count and read the baseline from EEPROM.
  powercycles = (0);
  EEPROM.write(0, powercycles);
  EEPROM.get(baseline_address, read_baseline);
  
  // Blink LED's green to indicate that the altimeter is running.
  blinkLEDcolor(green, 100, 100);
    
}

void loop() {
  #ifdef simulation
    agl = agl - 100;
    delay(500);
  #else
    agl = bmp.readAltitudeSTDdm() - read_baseline.field1;
  #endif

  // Light up or blink the LEDs in different colors depending on altitude.
  if (agl > 3500) {
    setLEDcolor(blue);
  }
  else if (agl >= 3000) {
    blinkLEDcolor(blue, 800, 800);
  }
  else if (agl >= 2500) {
    setLEDcolor(green);
  }
  else if (agl >= 2000) {
    blinkLEDcolor(green, 800, 800);
  }
  else if (agl >= 1500) {
    setLEDcolor(yellow);
  }
  else if (agl >= 1000) {
    blinkLEDcolor(red, 800, 800);
  }
  else if (agl >= 700) {
    setLEDcolor(red);
  }
  else {
    setLEDcolor(off);
  } 

}
