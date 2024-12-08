#include "WiFi.h"
#include "ESPAsyncWebServer.h"

#define LED_PIN  18
#define SERVER_PORT 4080

/* Only defines:
 * CREDS_SSID
 * CREDS_PASS */
#include "creds.h"

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
WiFiServer TCPserver(SERVER_PORT);

String readTemp() {
  return String("10C");
}

String readHumi() {
  return String("10%");
}

String readPres() {
  return String("1000hPA");
}

void setup(){
  Serial.begin(115200);
  
  pinMode(LED_PIN, OUTPUT);
  
  Serial.println("ESP32 server");
  Serial.print("Setting AP (Access Point)…");
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readTemp().c_str());
  });

  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readHumi().c_str());
  });

  server.on("/pressure", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readPres().c_str());
  });
  
  bool status;
  
  // Start server
  server.begin();
  TCPserver.begin();
}
 
void loop(){
  WiFiClient client = TCPserver.available();

  if (client) {
    // Read the command from the TCP client:
    char command = client.read();
    Serial.print("ESP32 #2: - Received command: ");
    Serial.println(command);

    if (command == '1')
      digitalWrite(LED_PIN, HIGH); // Turn LED on
    else if (command == '0')
      digitalWrite(LED_PIN, LOW);  // Turn LED off

    client.stop();
  }
}