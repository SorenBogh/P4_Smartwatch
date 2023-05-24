#include <HardwareSerial.h>

#define RESETLINE 33  // Change this if you are not using an Arduino Adaptor Shield Version 2 (see code below)
#define RXp2 25 // Sets the TX and RX pins to pin 25 and 26. This is necessary as the standard pins for an esp32 are used for the built in display on the ttgo t1 board
#define TXp2 26

static uint8_t conv2d(const char* p) {
  uint8_t v = 0;
  if ('0' <= *p && *p <= '9')
    v = *p - '0';
  return 10 * v + *++p - '0';
}

uint8_t hh=conv2d(__TIME__), mm=conv2d(__TIME__+3), ss=conv2d(__TIME__+6); 


void setup() {
 Serial.begin(115200); // sets standard serial allowing for serial monitoring
 Serial2.begin(115200, SERIAL_8N1, RXp2, TXp2); // sets the baud rate and the pins for serial2, which is used to communicate with the display. The baudrate needs to match the displays baud rate.
 // Serial.begin(9600, RXD2, TXD2);
pinMode(RESETLINE, OUTPUT);  // Set D4 on Arduino to Output (4D Arduino Adaptor V2 - Display Reset)
  digitalWrite(RESETLINE, 0);  // Reset the Display via pin 33
  delay(100);
  digitalWrite(RESETLINE, 1);  // unReset the Display via pin 33
  // Let the display start up after the reset (This is important)
  // Increase to 4500 or 5000 if you have sync problems as your project gets larger. 
  delay (5000); 

Serial.println("sending time");
Serial2.print("a");
if(hh < 10){
Serial2.print("0");
}
Serial2.print(hh);
if(mm < 10){
Serial2.print("0");
}
Serial2.print(mm);
//Serial2.print(ss);
//Serial2.print("*");
Serial.println("time sent");

for(int i=15; i >0; i--){
Serial2.print("b");
Serial2.print(i);
Serial2.print("*");
delay(1000);
}
for(int i=0; i <15; i++){
Serial2.print("b");
Serial2.print(i);
Serial2.print("*");
delay(1000);
}

}
void loop() {
}

