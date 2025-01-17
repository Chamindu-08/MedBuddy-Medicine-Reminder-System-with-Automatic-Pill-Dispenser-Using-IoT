#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Initialize the library by specifying the LCD address (0x27 for a 16x2 LCD)
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() 
{
    lcd.begin(); // Initialize the LCD
    lcd.backlight(); // Turn on the backlight
}

void loop() 
{
    lcd.setCursor(0, 0); // Set cursor to first row, first column
    lcd.print(" Tech Maker "); // Print 16 characters
    delay(1000); // Delay for 1 second

    lcd.setCursor(0, 1); // Set cursor to second row, first column
    lcd.print("Like | Share"); // Print message
    delay(8000); // Delay for 8 seconds

    lcd.clear(); // Clear the display
    lcd.setCursor(0, 0); // Set cursor to first row, first column
    lcd.print(" SUBSCRIBE "); // Print message
    lcd.setCursor(0, 1); // Set cursor to second row, first column
    lcd.print(" TECH MAKER "); // Print message
    delay(8000); // Delay for 8 seconds
}
