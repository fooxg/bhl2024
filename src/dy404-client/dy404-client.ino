/*
 * Dywizjon 404
 * Adam G., Maciej K., Rafal L.
 * ESP8266 client for monitoring life support
 */

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <SPI.h>
#include <SD.h>
#include <DHT.h>

/* Only defines:
 * CREDS_SSID
 * CREDS_PASS */
#include "creds.h"

//const int MOSI = 11;
//MISO = 12;
//CLK  = 13;
//SPI.begin();
const int CS   = 4;
String serverAddress = "192.168.4.1";
String serverName = "http://192.168.4.1:80/getClientID";
const int   serverPort    = 4080;

WiFiClient TCPclient;
HTTPClient http;

DHT dht(0,DHT11);

float temp;
float humidity;

uint8_t id;
char name[16] = "Operator";
float presure  = 1.0*random(9600,10400)/10.0;
uint8_t battery    = random(0, 255);
uint8_t saturation = random(0, 255);
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
  Serial.println("");
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

void init_blackbox() {
  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  while(!SD.begin(CS)) {
    Serial.println("Card failed, or not present");
    delay(1000);
    // TODO handle event by initialising network first
    // and send info to engineer panel
  }
  Serial.println("card initialized.");
}

void log_to_blackbox(String text) {
  // SD handling
  // TODO RTC
  File dataFile = SD.open("client_log", FILE_WRITE);
  if (dataFile) {
    dataFile.println(text);
    dataFile.close();
  } else {
    Serial.println("failed to log data"); 
  }
}

void setup() {
  Serial.begin(9600);

  Serial.println("ESP8266 belt client");
  
  init_blackbox();
  init_network();
  dht.begin();

  id = get_id();
  Serial.println(id);

  Serial.println("Network stack initialised");
}

void gather_sensors() {
  // TODO
  // all sensor data here
  temp = dht.readTemperature();
  humidity = dht.readHumidity();
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

  mess[27] = (uint8_t)humidity;

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
    Serial.println("Disconnected from host");
    TCPclient.stop();
    // Log to blackbox only if cannot connect
    while (!TCPclient.connect(serverAddress, serverPort)) {
      gather_sensors();
      log_to_blackbox("data");
      delay(500);
    }
    Serial.println("Connection to host reestablished");
  }

  gather_sensors();
  log_to_blackbox("data");
  send_to_host();

  delay(500);
}