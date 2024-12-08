/*
 * Dywizjon 404
 * Adam G., Maciej K., Rafal L.
 * ESP8266 client for monitoring life support
 */

#include <ESP8266WiFi.h>
#include <SPI.h>
#include <SD.h>
#include <DHT.h>
#include <RtcDS1302.h>

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
#define DHTPIN 0
#define DHTTYPE DHT11
ThreeWire myWire(4,2,5); // IO, SCLK, CE
RtcDS1302<ThreeWire> Rtc(myWire);
DHT dht(DHTPIN, DHTTYPE);

float temp;
float humidity;

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
  Rtc.Begin();
  dht.begin();
  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  Serial.println("Network stack initialised");
}

void gather_sensors() {
  // TODO
  // all sensor data here
  temp = dht.readTemperature();
  humidity = dht.readHumidity();
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
        RtcDateTime now = Rtc.GetDateTime();
Serial.println(dht.readTemperature());
    printDateTime(now);
    Serial.println();
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

#define countof(a) (sizeof(a) / sizeof(a[0]))

void printDateTime(const RtcDateTime& dt)
{
    char datestring[26];

    snprintf_P(datestring, 
            countof(datestring),
            PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
            dt.Month(),
            dt.Day(),
            dt.Year(),
            dt.Hour(),
            dt.Minute(),
            dt.Second() );
    Serial.print(datestring);
}
