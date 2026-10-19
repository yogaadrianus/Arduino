const int sensorPin = A0;
int moistureValue;

void setup() {
  Serial.begin(9600);
}

void loop() {
  moistureValue = analogRead(sensorPin);

  Serial.print("Soil Moisture Value: ");
  Serial.println(moistureValue);

  if (moistureValue > 700) {
    Serial.println("Soil is DRY");
  }
  else if (moistureValue > 400) {
    Serial.println("Soil is MODERATE");
  }
  else {
    Serial.println("Soil is WET");
  }

  Serial.println("--------------------");
  delay(1000);
}