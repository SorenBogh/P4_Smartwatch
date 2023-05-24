#include <Wire.h>

float Temp;
int16_t tempData;


void internalTemp() {
  //Getting measurements from Temperature Sensor
  Wire.beginTransmission(0x68);
  Wire.write(0x41);
  Wire.endTransmission();
  Wire.requestFrom(0x68, 2);
  tempData = Wire.read() << 8 | Wire.read();
  Temp = ((float)tempData / 340) + 36.53;      //Calculate measurement to Celsius
  
    if (Temp > 80) {
      Serial.print(Temp);
      Serial.print(" ");
      Serial.println("Turning Off the Watch. Watch is too hot"); 
      Serial2.println("b , 0");    //Sends that the Screen.
      deepSleep();                 //Starts Deep sleep.  
    } 
    else {
      Serial.print(Temp);
      Serial.print(" ");
      Serial.println(" Watch not burning");
      delay(1000);
    }
  }

void deepSleep(){
  esp_deep_sleep_start();                     //ESP32 goes to deep sleep mode.
  esp_sleep_enable_timer_wakeup(30000000);    //Wakes up after 30 Sec.
  tempDeep();                                 //Enters tempDeep.
}

void tempDeep(){
//Check if the temperature is low enough.
  Wire.beginTransmission(0x68);
  Wire.write(0x41);
  Wire.endTransmission();
  Wire.requestFrom(0x68, 2);
  tempData = Wire.read() << 8 | Wire.read();
  Temp = ((float)tempData / 340) + 36.53;      //Calculate measurement to Celsius

  if (Temp > 50) {
  deepSleep();
  }
  else {
  internalTemp();
  }
}

void setup() {
Wire.begin(21, 22);
  Serial.begin(115200);
  Wire.setClock(100000);
  delay(250);

}

void loop() {
  internalTemp();

}
