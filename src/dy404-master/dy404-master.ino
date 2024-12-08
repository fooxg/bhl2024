/*
 * Dywizjon 404
 * Adam G., Maciej K., Rafal L.
 * Master controller for esp32
 */

#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Keypad.h>
#include <math.h>

/* Only defines:
 * CREDS_SSID
 * CREDS_PASS */
#include "creds.h"

// OLED
#define WIDTH 128
#define HEIGHT 64
#define MIDLINE 8+(HEIGHT-8)/2
#define OLED_ADDR   0x3C

#define SERVER_PORT 4080

//init struts
typedef struct time{
  uint8_t hour;
  uint8_t min;
} Time;

typedef struct userData{
  char name[16];
  uint8_t battery;
  uint16_t presure;
  uint16_t heart_rate;
  uint8_t C02;
  uint8_t saturation;
  uint8_t humidity;
} UserData;

void draw_main_interface(Time, UserData);

/* ------------ keypad ------------ */
const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
//define the cymbols on the buttons of the keypads
char hexaKeys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','5','6','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {2,0,4,16}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {17,5,18,19}; //connect to the column pinouts of the keypad

//initialize an instance of class NewKeypad
Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 
/* ------------ end of keypad ------------ */

Adafruit_SSD1306 display(WIDTH, HEIGHT, &Wire, -1);

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
WiFiServer TCPserver(SERVER_PORT);

//init global variables
Time stime = {21,38};
UserData GuideData = {"Guide",64,1030,120,2,90,34};
UserData GuestData1 = {"Adam",87,1040,117,3,92,30};
UserData GuestData2 = {"Maciek",100,1027,90,4,87,26};
UserData GuestData3 = {"Rafal",34,1065,99,1,94,15};
UserData ActualData = GuideData;
uint8_t sec = 0;
uint8_t battery = 64;

void init_networking() {
  Serial.print("Setting AP (Access Point)…");
  WiFi.softAP(CREDS_SSID, CREDS_PASS);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  server.on("/getClientID", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", assign_id().c_str());
  });

  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readHumi().c_str());
  });

  server.on("/pressure", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readPres().c_str());
  });
  
  //TODO add LITTTLEFS

  // Start servers
  server.begin();
  TCPserver.begin();
}

void setup(){
  Serial.begin(9600);
  
  Serial.println("ESP32 hand band");
  
  init_networking();

  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  customKeypad.addEventListener(keypadEvent);
}

void loop(){
  WiFiClient client = TCPserver.available();

  if (client) {
    // Read the command from the TCP client:
    char command = client.read();
    //Serial.println(command);
    client.stop();
  }
  //char key = customKeypad.getKey();
  draw_main_interface(stime,ActualData);
}

// Handle drawing
void draw_main_interface(Time time,UserData userData){
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  //write upper line
  display.setCursor(0, 0);
  display.print(time.hour);
  display.print(":");
  display.print(time.min);
  display.setCursor(WIDTH/2-(strlen(userData.name)*3),0);
  display.print(userData.name);
  if(userData.battery < 100){
    display.setCursor(WIDTH-15-3, 0);
    display.print(userData.battery);
    display.print("%");
  } else {
    display.setCursor(WIDTH-20-4, 0);
    display.print(userData.battery);
    display.print("%");
  }
  //
  display.setCursor(0, 9);
  display.print("Saturation");
  display.setCursor(WIDTH/2+2, 9);
  display.print("Presure");
  display.setCursor(0,MIDLINE+1);
  display.print("Heart rate");
  display.setCursor(WIDTH/2+2, MIDLINE+1);
  display.print("CO2");

  display.setCursor(WIDTH/2-((int)log10(userData.saturation)+1)*12, 20);
  display.setTextSize(2);
  display.print(userData.saturation);
  display.setCursor(WIDTH-((int)log10(userData.presure)+1)*12, 20);
  display.print(userData.presure);
  display.setCursor(WIDTH/2-((int)log10(userData.heart_rate)+1)*12,MIDLINE+12);
  display.print(userData.heart_rate);
  display.setCursor(WIDTH-((int)log10(userData.C02)+1)*12, MIDLINE+12);
  display.print(userData.C02);

  display.drawLine(0, 8, WIDTH, 8, WHITE);
  display.drawLine(WIDTH/2, 9, WIDTH/2, HEIGHT, WHITE);
  display.drawLine(0,MIDLINE,WIDTH,MIDLINE,WHITE);
  display.display();
}

void keypadEvent(KeypadEvent key){
  if(customKeypad.getState() == PRESSED){
    switch (key){
    case '1':
      ActualData = GuideData;
      break;
    case '2':
      ActualData = GuestData1;
      break;
    case '3':
      ActualData = GuestData2;
      break;
    case '4':
      ActualData = GuestData3;
      break;
    default:
      break;
    }
  }
}

String assign_id() {
  static int i=0;
  return String(i++);
}

String readHumi() {
  return String("10%");
}

String readPres() {
  return String("1000hPA");
}