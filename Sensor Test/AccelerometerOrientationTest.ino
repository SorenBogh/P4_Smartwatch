#include <Wire.h>


const uint8_t sda = 21;
const uint8_t scl = 22;

int16_t AccXdata, AccYdata, AccZdata;
float AccX, AccY, AccZ;
float Roll, Pitch, Yaw;
float r2D = (3.14/180); //Radian to Degrees

void watchOrientation(void) {
  
  //Getting measurements from Accelerometer
  Wire.beginTransmission(0x68);
  Wire.write(0x3B);
  Wire.endTransmission(); 
  Wire.requestFrom(0x68,6);
  AccXdata = Wire.read() * 256 | Wire.read();
  AccYdata = Wire.read() * 256 | Wire.read();
  AccZdata = Wire.read() * 256 | Wire.read();

  AccX = (float)AccXdata/16384;
  AccY = AccYdata;
  AccZ = AccZdata;

    Pitch = asin(AccX/1)/r2D;    //Calculation for Pitch.
    Roll = atan(AccY/AccZ)/r2D;  //Calculation for Roll.
  

  if ((Roll <=30) && (Roll >=-45) && (Pitch <=30) && (Pitch >=-45) && ((float)AccZ/16384 > 0)) {
    Serial.println("Turning on Screen");
  }
  else {
    Serial.println("Turning off Screen");
  }
}

void setup() {
  Wire.begin(sda, scl);
  Serial.begin(115200);
  Wire.setClock(100000);
  delay(250);

  Wire.beginTransmission(0x68); 
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission();

  //Choosing accelerometer' range 
  Wire.beginTransmission(0x68);
  Wire.write(0x1C);
  Wire.write(0x0);
  Wire.endTransmission();

  Wire.beginTransmission(0x68);
  Wire.write(0x6C);
  Wire.write(0x47);
  Wire.endTransmission();

}

void loop() {
  watchOrientation();
  Serial.print("  ");
  Serial.print("Roll=");
  Serial.print(Roll);
  Serial.print("  ");
  Serial.print("Pitch=");
  Serial.println(Pitch);
  delay(500);
}