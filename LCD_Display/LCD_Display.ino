#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
    Wire.begin(4, 5); // SDA = GPIO4 (D2), SCL = GPIO5 (D1)

    lcd.begin();
    lcd.backlight();
}

void loop() {
    lcd.setCursor(0, 0);
    lcd.print(" MedBuddy ");
    delay(1000);

    lcd.setCursor(0, 1);
    lcd.print("Group 02");
    delay(2000);
}
