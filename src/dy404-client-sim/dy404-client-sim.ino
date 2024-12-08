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

uint8_t id;
char name[16] = "Jestem";
float temp     = 1.0*random(0, 400)/10.0;
float presure  = 1.0*random(9600,10400)/10.0;
uint8_t battery    = random(0, 255);
uint8_t saturation = random(0, 255);
uint8_t humidity   = random(0, 255);
uint8_t heart_rate = random(0, 255);
uint16_t C02       = random(0, 400);
uint32_t rtctime   = 1733641479;


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

  presure  += 1.0*random(0,20)/10.0-1.0;
  if (presure>1100.0)
    presure=1090.0;
  if (presure<900.0)
    presure=910.0;

  saturation += random(0, 2)-1;
  if (saturation>253)
    saturation=250;
  if (saturation<2)
    saturation=5;
  
  humidity += random(0, 2)-1;
  if (humidity>253)
    humidity=250;
  if (humidity<2)
    humidity=5;

  heart_rate += random(0, 2)-1;
  if (heart_rate>140)
    heart_rate=138;
  if (heart_rate<40)
    heart_rate=42;

  rtctime += 1;
}

void send_to_host() {
  byte mess[35];
  mess[0] = id;

  for (int i{0}; i<16; i++)
    mess[1+i]=name[i];

  mess[17] = ((uint32_t)temp >> 24);
  mess[18] = ((uint32_t)temp >> 16) & 0xFF;
  mess[19] = ((uint32_t)temp >> 8) & 0xFF;
  mess[20] = (uint32_t)temp & 0xFF;
  
  mess[21] = ((uint32_t)presure >> 24);
  mess[22] = ((uint32_t)presure >> 16) & 0xFF;
  mess[23] = ((uint32_t)presure >> 8) & 0xFF;
  mess[24] = (uint32_t)presure & 0xFF;
  
  mess[25] = battery;

  mess[26] = saturation;

  mess[27] = humidity;

  mess[28] = heart_rate;

  mess[29] = C02 >> 8;
  mess[30] = C02 & 0xFF;

  mess[31] = (rtctime >> 24);
  mess[32] = (rtctime >> 16) & 0xFF;
  mess[33] = (rtctime >> 8) & 0xFF;
  mess[34] = rtctime & 0xFF;

  TCPclient.write(mess,35);
  TCPclient.flush();
}

void loop() {
  // Check connection
  if (!TCPclient.connected()) {
    TCPclient.stop();
    // Log to blackbox only if cannot connect
    while (!TCPclient.connect(serverAddress, serverPort)) {
      delay(200);
    }
  }

  simulate_sensors();
  send_to_host();

  delay(500);
}