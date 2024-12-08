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
  uint8_t id;
  char name[16];
  float temp;
  float presure;
  uint8_t battery;
  uint8_t saturation;
  uint8_t humidity;
  uint8_t heart_rate;
  uint16_t C02;
  uint32_t rtctime;
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
int choice=0;
int max_c=0;
UserData user_array[5];

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
  Serial.println("Setup complete");
}

void loop(){
  WiFiClient client = TCPserver.available();

  if (client) {
    uint8_t mess[35]; 
    int len = client.read(mess, 35);
    int id = mess[0];
    if (id < max_c) {
      user_array[id].id = mess[0];
      for (int i{0}; i<16; i++)
        user_array[id].name[i] = mess[1+i];

      uint32_t temp;
      temp = mess[20];
      temp |= mess[19]<<8;
      temp |= mess[18]<<16;
      temp |= mess[17]<<24;
      user_array[id].temp = ((float)temp);

      temp = mess[24];
      temp |= mess[23]<<8;
      temp |= mess[22]<<16;
      temp |= mess[21]<<24;
      user_array[id].presure = ((float)temp);

      user_array[id].battery = mess[25];

      user_array[id].saturation = mess[26];

      user_array[id].humidity = mess[27];

      user_array[id].heart_rate = mess[28];

      uint16_t CO2;
      CO2 = mess[30];
      CO2 |= mess[29];
      user_array[id].C02 = CO2;
      temp = mess[34];
      temp |= mess[33]<<8;
      temp |= mess[32]<<16;
      temp |= mess[31]<<24;
      user_array[id].rtctime = temp;
    }
    Serial.print(user_array[id].id);
    Serial.print(" ");
    Serial.print(id);
    Serial.print(" ");
    Serial.println(user_array[id].rtctime);
    client.stop();
  }
  char key = customKeypad.getKey();
  Time stime = {21, 37};
  if (max_c > 0) {
    draw_main_interface(stime, user_array[choice]);
  }
}

// Handle drawing
void draw_main_interface(Time time, UserData userData){
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
  display.print((int)userData.presure);
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
      choice = 1;
      break;
    case '2':
      choice = 2;
      break;
    case '3':
      choice = 3;
      break;
    case '4':
      choice = 4;
      break;
    default:
      choice = 0;
      break;
    }
  }
  if (choice >= max_c)
    choice = max_c-1;
  if (choice < 0)
    choice = 0;
}

String assign_id() {
  static int i=0;
  max_c = 214;
  return String(i++);
}

String readHumi() {
  return String("10%");
}

String readPres() {
  return String("1000hPA");
}