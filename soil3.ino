#define WATER_PUMP 5
#define SENSOR A0

float sensorValue = 0;

void setup() {
  pinMode(WATER_PUMP, OUTPUT);
  pinMode(SENSOR, INPUT);
  Serial.begin(9600);
}

void loop() {
  sensorValue = 0;

  for (int i = 0; i < 100; i++) {
    sensorValue = sensorValue + analogRead(SENSOR);
    delay(1);
  }

  sensorValue = sensorValue / 100.0;

  Serial.print("Sensor Value: ");
  Serial.println(sensorValue);

  if (sensorValue >= 512) {
    digitalWrite(WATER_PUMP, HIGH);
    Serial.println("Soil is DRY - Pump ON");
  }
  else {
    digitalWrite(WATER_PUMP, LOW);
    Serial.println("Soil is WET - Pump OFF");
  }

  delay(30);
}