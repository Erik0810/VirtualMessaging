#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// You might need to scan for the actual I2C address if this one doesn't work
LiquidCrystal_I2C lcd(0x27, 16, 2);  // (address, columns, rows)

void setup() {
  Wire.begin(21, 22);  // SDA = 21, SCL = 22 (for ESP32)
  lcd.begin(16,2);
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Hello ESP32!");
}

void loop() {
  // Nothing here
}