#define BUTTON_PIN D4

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  Serial.begin(9600);
}

void loop() {
  if (digitalRead(BUTTON_PIN) == HIGH) {
    Serial.println("Emergency Alert Triggered!");
  }
}
