#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Keypad.h>
//init defines
#define WIDTH 128
#define HEIGHT 64
#define OLED_ADDR   0x3C

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

//init own functions
void data_abot_guests();
void draw_main_interface(Time,UserData);

//init keypad
const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
//define the cymbols on the buttons of the keypads
char hexaKeys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','5','6','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {2,0,4,16};; //connect to the row pinouts of the keypad
byte colPins[COLS] = {17,5,18,19}; //connect to the column pinouts of the keypad

//initialize an instance of class NewKeypad
Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 
//end of keypad

Adafruit_SSD1306 display(WIDTH, HEIGHT, &Wire, -1);
//init global variables
Time stime = {21,37};
UserData GuideData = {"Guide",64,1030,120,2,90,34};
UserData GuestData1 = {"Adam",87,1040,117,3,92,30};

uint8_t sec = 0;
uint8_t battery = 64;
void setup() {
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  Serial.begin(9600);
  draw_main_interface(stime,GuideData);
}

void loop() {
  char customKey = customKeypad.getKey();
  
  if (customKey){
    switch(customKey){
      case '1':
          draw_main_interface(stime,GuideData);
          break;
      case '2':
          draw_main_interface(stime,GuestData1);
          break;
    }
  }
}

void data_abot_guests(){
  char customKey = customKeypad.getKey();
  

}
void draw_main_interface(Time time,UserData userData){
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(time.hour);
  display.print(":");
  display.print(time.min);
  display.print("  ");
  display.print(userData.name);
  display.print("  ");
  display.print(strlen(userData.name));
  display.setCursor(WIDTH-15-3, 0);
  display.print(userData.battery);
  display.print("%");
  //display.drawRect(int16_t x, int16_t y, int16_t w, int16_t h, white)
  display.drawLine(0, 8, WIDTH, 8, WHITE);
  display.drawLine(WIDTH/2, 8, WIDTH/2, HEIGHT, WHITE);
  display.display();
}