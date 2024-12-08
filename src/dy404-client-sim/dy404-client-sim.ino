/*
 * Dywizjon 404
 * Adam G., Maciej K., Rafal L.
 * ESP8266 dummy client for simulating life support
 */

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

/* Only defines:
 * CREDS_SSID
 * CREDS_PASS */
#include "creds.h"

String serverAddress = "192.168.4.1";
String serverName = "http://192.168.4.1:80/getClientID";
const int   serverPort    = 4080;

WiFiClient TCPclient;
HTTPClient http;

int id;
char name[16] = "Jestem testem";
uint8_t battery = random(0, 255);
float temp = 1.0*random(0, 400)/10.0;
uint16_t C02 = random(0, 400);
uint8_t saturation;
uint8_t humidity;

uint16_t presure;
uint16_t heart_rate;
uint32_t rtctime;


void init_network() {
  // connect to Wi-Fi
  WiFi.begin(CREDS_SSID, CREDS_PASS);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("Connected to WiFi");

  // connect to controller system
  while(!TCPclient.connect(serverAddress, serverPort)) {
    Serial.println("Failed to connect to host, retrying in 1s");
    delay(1000);
  }
  Serial.println("Connected to host");
}

int get_id() {
  http.begin(TCPclient, serverName.c_str());
  int httpResponseCode = http.GET();
  String payload = http.getString();
  http.end();
  return payload.toInt();
}

void setup() {
  Serial.begin(9600);

  Serial.println("ESP8266 belt client simulator");

  init_network();

  id = get_id();
  Serial.println(id);
  Serial.println("Network stack initialised");
}

void simulate_sensors() {
  battery -= (random(10)>5) ? 1 : 0;
  if (battery==2)
    battery=10;

  temp += random(0,30)/10.0-0.5;
  if (temp>40.0)
    temp=38.0;
  if (temp<5.0)
    temp=7.0;

  C02 += random(0,30)-15;
  if (C02>400)
    C02=380;
  if (C02<20)
    C02=50;

}

void send_to_host() {
  // daaaataaa neeeded
  TCPclient.write('1');
  TCPclient.flush();
}

void loop() {
  // Check connection
  if (!TCPclient.connected()) {
    Serial.println("Disconnected from host");
    TCPclient.stop();
    // Log to blackbox only if cannot connect
    while (!TCPclient.connect(serverAddress, serverPort)) {
      delay(200);
    }
    Serial.println("Connection to host reestablished");
  }

  //simulate_sensors();
  send_to_host();

  delay(500);
}