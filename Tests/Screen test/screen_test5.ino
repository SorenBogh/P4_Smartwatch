#include <HardwareSerial.h>

#define RESETLINE 33  // Change this if you are not using an Arduino Adaptor Shield Version 2 (see code below)
#define RXp2 25 // Sets the TX and RX pins to pin 25 and 26. This is necessary as the standard pins for an esp32 are used for the built in display on the ttgo t1 board
#define TXp2 26

uint8_t wind = 1;
uint16_t water = 16;
float retSek = 13.36;
char str[]  = {"This is Thoaar"};


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


Serial2.print("ui8: ");
Serial.print("ui8: ");
delay (500);
Serial2.print(wind);
Serial.print(wind);
delay (500);
Serial2.print("\n\n");
Serial.print("\n\n");
delay (1000);
Serial2.print("ui16: ");
Serial.print("ui16: ");
delay (500);
Serial2.print(water);
Serial.print(water);
delay (500);
Serial2.print("\n\n");
Serial.print("\n\n");

delay (1000);
Serial2.print("float: ");
Serial.print("float: ");
delay (500);
Serial2.print(retSek);
Serial.print(retSek);
delay (500);
Serial2.print("\n\n");
Serial.print("\n\n");
delay (1000);
Serial2.print("str: ");
Serial.print("str: ");
delay (500);
for(int i = 0; i<sizeof(str); i++)
{
Serial2.print(str[i]);
Serial.print(str[i]);
delay (100);
}
delay (500);
Serial2.print("\n\n");
Serial.print("\n\n");
delay (1000);
Serial2.print("strv: ");
Serial.print("strv: ");
delay (500);

char buff[10];
char buff2[10];
char text[100] = {"This data is: "};

dtostrf(retSek,4,2,buff);
dtostrf(wind,2,0,buff2);
strcat(text,buff);
strcat(text, "\n      and this is: ");
strcat(text,buff2);
//Serial2.printf("The data is %u with %f \n", wind, retSek);
for(int i = 0; i<sizeof(text); i++)
{
Serial2.print(text[i]);
Serial.print(text[i]);
delay (100);
}


}

void loop() {
  // put your main code here, to run repeatedly:

}
