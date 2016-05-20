#define LED_PIN 3
#define LIGHT_PIN A0

int value;
void setup() {
  Serial.begin(9600);
  Serial.println("Example number 2");
}

void loop() {
  value = analogRead(LIGHT_PIN);
  int l = map(value, 100, 700, 255, 0);
  analogWrite(LED_PIN, l);
  delay(20);
}
