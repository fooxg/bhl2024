#include <ESP8266WiFi.h>
#include <SPI.h>
#include <SD.h>

/* Only defines:
 * CREDS_SSID
 * CREDS_PASS */
#include "creds.h"

//const int MOSI = 11;
//MISO = 12;
//CLK  = 13;
//SPI.begin();
const int CS   = 4;
const char* serverAddress = "192.168.4.1";
const int   serverPort    = 4080;

WiFiClient TCPclient;

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
  File dataFile = SD.open("client_log", FILE_WRITE);
  if (dataFile) {
    dataFile.println(text);
    dataFile.close();
  } else {
    Serial.println("failed to log data"); 
  }
}

void setup() {
  Serial.begin(115200);

  Serial.println("ESP8266 belt client");
  
  init_blackbox();
  init_network();

  Serial.println("Network stack initialised");
}

void gather_sensors() {
  // TODO
  // all sensor data here
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