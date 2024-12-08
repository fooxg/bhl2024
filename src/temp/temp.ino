#include<DHT.h>

DHT dht(5,DHT11);

void setup() {
  // Start the Serial Monitor
  Serial.begin(9600);
  dht.begin();
  delay(200);
}

void loop() {
  float temp = dht.readTemperature();
  float humidity = dht.readHumidity();
  Serial.print("temp: ");
  Serial.print(temp);
  Serial.print(", humidity: ");
  Serial.println(humidity);
  delay(1000);
}