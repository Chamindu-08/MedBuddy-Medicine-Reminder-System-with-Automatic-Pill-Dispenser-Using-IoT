#include <Servo.h>

//create a Servo object
Servo myServo;

void setup() {
  myServo.attach(10);
}

void loop() {
  //sweep from 0 to 180 degrees
  for (int i = 0; i <= 180; i++) {
    myServo.write(i);
    delay(15);
  }
  delay(2000);

  //sweep from 180 to 0 degrees
  for (int i = 180; i >= 0; i--) {
    myServo.write(i);
    delay(15);
  }
  delay(2000);
}
