int counter = 0;

void setup() {
  Serial.begin(9600);
  Serial.println("*** ROUND BEGIN ***");
}

void loop() {
  Serial.println(counter);
  delay(1000 - (counter * 100));
  counter = counter + 1;

  if (counter > 10) {
    counter = 0;
    Serial.println("*** ROUND BEGIN ***");
  }
}

