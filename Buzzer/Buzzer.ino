void setup() {
  pinMode(D5, OUTPUT);
}

void loop() {
  digitalWrite(D5, 1);
  delay(1000);
  digitalWrite(D5, 0);
  delay(1000);
}
