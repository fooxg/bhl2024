#include <RtcDS1302.h>
#include <ThreeWire.h>
ThreeWire myWire(48, 50, 52);        // rst, data, clk
RtcDS1302<ThreeWire> Rtc(48,50,52);    // RTC Object
void setup() {
  // Start the Serial Monitor
  Serial.begin(9600);
   Rtc.Begin();
  delay(200);
  RtcDateTime currentTime = RtcDateTime(__DATE__ , __TIME__);
  Rtc.SetDateTime(currentTime);
}

void loop() {
  RtcDateTime now = Rtc.GetDateTime();
  Serial.print("day: ");
  Serial.print(now.Hour());
  Serial.print(", minutey: ");
  Serial.print(now.Minute());
  Serial.print(", second: ");
  Serial.println(now.Second());

  delay(1000);
}