#include <Wire.h>


const uint8_t sda = 21;
const uint8_t scl = 22;

int16_t AccX, AccY, AccZ;
int16_t gyroX, gyroY, gyroZ;
float Roll, Pitch;
float rollStart, pitchStart;
float r2D = (3.14/180); //Radian to Degrees

void watchOrientation(void) {
  
  //Getting measurements from Accelerometer
  Wire.beginTransmission(0x68);
  Wire.write(0x3B);
  Wire.endTransmission(); 
  Wire.requestFrom(0x68,6);
  AccX = Wire.read() * 256 | Wire.read();
  AccY = Wire.read() * 256 | Wire.read();
  AccZ = Wire.read() * 256 | Wire.read();
  
  (float)AccX;
  (float)AccY;
  (float)AccZ;
  
  Roll=atan(AccY/sqrt(pow(AccX,2)+pow(AccZ,2)))/(r2D);
  Pitch=-atan(AccX/sqrt(pow(AccY,2)+pow(AccZ,2)))/(r2D);
/*

  rollStart=Roll;
  pitchStart=Pitch;

  //Getting measurements from Gyroscope
  Wire.beginTransmission(0x68);
  Wire.write(0x43);
  Wire.endTransmission();
  Wire.requestFrom(0x68, 6);
  gyroX = Wire.read() * 256 | Wire.read();
  gyroY = Wire.read() * 256 | Wire.read();
  gyroZ = Wire.read() * 256 | Wire.read(); 

  gyroX = gyroX / 65.5;
  gyroY = gyroY / 65.5;
  gyroZ = gyroZ / 65.5;   

*/
/*  if ((Roll <=30) && (Roll >=-45) && (Pitch <=30) && (Pitch >=-45) && (Yaw >= 30)) {
    Serial.println("Turning on Screen");
  }
  else {
    Serial.println("Turning off Screen");
  }
*/
}
void internalTemp(void) {
  //Getting measurements from Temperature Sensor
  Wire.beginTransmission(0x68);
  Wire.write(0x41);
  Wire.endTransmission();
  Wire.requestFrom(0x68,2);
  int16_t temp = Wire.read() << 8 | Wire.read();
  
  Serial.println(temp);

  if (temp > 80)  {
    Serial.println("Turning Off the Watch");
  }
    else {
    Serial.println("Keeping the watch on");
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

  //Choosing Gyroscope' range 
  Wire.beginTransmission(0x68);
  Wire.write(0x1B); 
  Wire.write(0x8);
  Wire.endTransmission();
}

void loop() {
  watchOrientation();
  Serial.print("Roll=");
  Serial.print(Roll);
  Serial.print("  ");
  Serial.print("Pitch=");
  Serial.print(Pitch);
  Serial.print("  ");
  Serial.print("Gyro X= ");
  Serial.print(gyroX);
  Serial.print("  ");
  Serial.print("Gyro Y= ");
  Serial.print(gyroY);
  Serial.print("  ");
  Serial.print("Gyro Z= ");
  Serial.println(gyroZ);
  delay(50);
}