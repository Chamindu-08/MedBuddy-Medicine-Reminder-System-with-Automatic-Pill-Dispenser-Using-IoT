#include <Wire.h>
#include <RTClib.h>

RTC_DS3231 rtc;

void setup() {
  Serial.begin(9600);

  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, setting the time!");

    // When the RTC loses power, it sets the time to January 1, 2000
    // You can set the time to the current time here, or adjust manually later
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  //Set the time and date manually
  //rtc.adjust(DateTime(2024, 6, 30, 23, 00, 0)); // Year, Month, Day, Hour, Minute, Second
}

void loop() {
  DateTime now = rtc.now();

  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(" ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();

  delay(1000);
}
