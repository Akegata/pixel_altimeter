/***************************************************************************
  This is a library for the BMP280 humidity, temperature & pressure sensor

  Designed specifically to work with the Adafruit BMEP280 Breakout 
  ----> http://www.adafruit.com/products/2651

  These sensors use I2C or SPI to communicate, 2 or 4 pins are required 
  to interface.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit andopen-source hardware by purchasing products
  from Adafruit!

  Written by Limor Fried & Kevin Townsend for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
 ***************************************************************************/

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
 
LiquidCrystal_I2C lcd(0x27,20,4);

#define BMP_SCK 13  // SCL (SCK)
#define BMP_MISO 12 // SDO
#define BMP_MOSI 11 // SDA (SDI)
#define BMP_CS 10   // CSB (CS)

//Adafruit_BMP280 bme; // I2C
//Adafruit_BMP280 bmp(BMP_CS); // hardware SPI
Adafruit_BMP280 bmp(BMP_CS, BMP_MOSI, BMP_MISO,  BMP_SCK);

int agl = 0;
int baseline_address    = 1; // Address in the EEPROM where the baseline reading should be stored.
int powercycles_address = 0;
int startup             = 0;
int powercycles;
double baseline;

struct eeprom_entry {
  double field1;
  byte field2;
  char name[10];
};
eeprom_entry read_baseline;
  
void setup() {
  Serial.begin(9600);
  Serial.println(F("BMP280 test"));
  
  if (!bmp.begin()) {  
    Serial.println("Could not find a valid BMP280 sensor, check wiring!");
    while (1);
  }
   
  lcd.init(); 
  lcd.backlight();
  if (!bmp.begin()) {
  lcd.print("ERROR, check wiring!");
    while (1) {}
  }
  lcd.clear();

  powercycles = (EEPROM.read(powercycles_address));
      lcd.print("Powercycles: ");
    lcd.print(powercycles);
  // On the third power cycle, reset the calibration
  if (powercycles == 2) {
    baseline = bmp.readAltitude();
    EEPROM.put(baseline_address, baseline);
  }
  // Update the powercycle count.
  else {
    powercycles++;
    EEPROM.write(powercycles_address, powercycles);
    delay(1000);
  }

  // Reset the power cycle count and read the baseline from EEPROM.
  powercycles = (0);
  EEPROM.write(0, powercycles);
  EEPROM.get(baseline_address, read_baseline);
}
  
void loop() {

    agl = bmp.readAltitude() - read_baseline.field1;
    //read pressure
    lcd.clear();
    lcd.setCursor(0, 0);

    lcd.print("Altitude: ");
    lcd.print(bmp.readAltitude(1025.3));
    lcd.print(" m");
        //read altitude
    lcd.setCursor(0, 1);
    lcd.print("Alt diff: ");
    lcd.print(agl);
    lcd.print(" m");
    
    Serial.print("Temperature = ");
    Serial.print(bmp.readTemperature());
    Serial.println(" *C");
    
    Serial.print("Pressure = ");
    Serial.print(bmp.readPressure());
    Serial.println(" Pa");

    Serial.print("Approx altitude = ");
    Serial.print(bmp.readAltitude(1025.3)); // this should be adjusted to your local forcase
    Serial.println(" m");
    
    Serial.println();
    delay(2000);
}
