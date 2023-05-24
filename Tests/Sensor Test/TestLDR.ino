int LDR;
const int LDRSensor = 12;
const int res = 12;

void Brightness () {
  analogReadResolution(res);
  LDR = analogRead(LDRSensor);

    if (LDR <= 1023) {
        Serial2.println("b , 3");
        Serial.print(LDR);
        Serial.print(" ");
        Serial.println("Brightness on: 3");
        }
        else if ((LDR > 1023) && (LDR <=2047)) {
        Serial2.println("b , 7");
        Serial.print(LDR);
        Serial.print(" ");
        Serial.println("Brightness on: 7");
        }
        else if ((LDR > 2047) && (LDR <=3071)) {
        Serial2.println("b , 11");
        Serial.print(LDR);
        Serial.print(" ");
        Serial.println("Brightness on: 11");
        }
        else {
        Serial2.println("b , 15");
        Serial.print(LDR);
        Serial.print(" ");
        Serial.println("Brightness on: 15");
        }  
    }

void setup() {
Serial.begin(115200);
}

void loop() {
  Brightness();
  delay(1000);
}
