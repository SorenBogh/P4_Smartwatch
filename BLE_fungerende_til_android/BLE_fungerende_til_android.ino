

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>


uint8_t type[] = { 0xAB, 0x00, 0x11, 0xFF, 0x92, 0xC0, 0x01, 0x01, 0x38, 0x81, 0x10, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0xA2, 0x00, 0x80 };  // id for DT78 app

#define SERVICE_UUID "6e400001-b5a3-f393-e0a9-e50e24dcca9e"
#define CHARACTERISTIC_UUID_RX "6e400002-b5a3-f393-e0a9-e50e24dcca9e"
#define CHARACTERISTIC_UUID_TX "6e400003-b5a3-f393-e0a9-e50e24dcca9e"

static BLECharacteristic* pCharacteristicTX;
static BLECharacteristic* pCharacteristicRX;

static bool deviceConnected = false;
static int id = 0;
long timeout = 10000, timer = 0;
bool  notify = true, b1;
int lines = 0, msglen = 0;

char msg[126];
String msg0, msg1, msg2, msg3, lastmessage = "";


class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
  }
  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    id = 0;
  }
};

class MyCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* pCharacteristic) {
    uint8_t* pData;
    std::string value = pCharacteristic->getValue();
    int len = value.length();
    pData = pCharacteristic->getData();
    if (pData != NULL) {
      if (pData[0] == 0xAB) {
        switch (pData[4]) {
          case 0x72:
            timer = millis();
            msglen = pData[2] - 5;
            lines = ceil(float(msglen) / 21);
            msg[msglen] = 0;
            if (pData[6] == 1) {
              //call
              timer = millis() + 15000;
              for (int x = 0; x < len; x++) {
                msg[x] = char(pData[x + 8]);
              }
            } else if (pData[6] == 2) {
              //cancel call
              timer = millis() - timeout;
            } else {
              //notification
              for (int x = 0; x < len; x++) {
                msg[x] = char(pData[x + 8]);
              }
            }
            break;
        }

      } else {
        switch (pData[0]) {
          case 0:
            for (int x = 0; x < len - 1; x++) {
              msg[x + 12] = char(pData[x + 1]);
            }
            break;
          case 1:
            for (int x = 0; x < len - 1; x++) {
              msg[x + 31] = char(pData[x + 1]);
            }
            break;
          case 2:
            for (int x = 0; x < len - 1; x++) {
              msg[x + 50] = char(pData[x + 1]);
            }
            break;
          case 3:
            for (int x = 0; x < len - 1; x++) {
              msg[x + 69] = char(pData[x + 1]);
            }
            break;
        }
      }
    }
  }
};

void initBLE() {
  BLEDevice::init("ESP32DAP2");
  BLEServer* pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService* pService = pServer->createService(SERVICE_UUID);
  pCharacteristicTX = pService->createCharacteristic(CHARACTERISTIC_UUID_TX, BLECharacteristic::PROPERTY_NOTIFY);
  pCharacteristicRX = pService->createCharacteristic(CHARACTERISTIC_UUID_RX, BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR);
  pCharacteristicRX->setCallbacks(new MyCallbacks());
  pCharacteristicTX->setCallbacks(new MyCallbacks());
  pCharacteristicTX->addDescriptor(new BLE2902());
  pCharacteristicTX->setNotifyProperty(true);
  pService->start();


  // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
  BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
}

void setup() {
  xTaskCreate(showNotification, "ble", 3500, NULL, 3, NULL);
  Serial.begin(115200);
  initBLE();
}

void loop() {

}

void showNotification(void* pvParameters) {
  while (1){
  String s = String(msg);
  copyMsg(s);
  if (s.length() == 0) {
    msg0 = "No messsage";
  } else {

    if (lastmessage != msg0) {
       
      int index = s.indexOf(':');
      if (index != -1) {
        String str = s.substring(0, index);
        Serial.print("Ny besked fra: ");
        Serial.println(str);
        
      }


      lastmessage = msg0;
    }
  }
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  }

}


void copyMsg(String ms) {
  switch (lines) {
    case 1:
      msg0 = ms.substring(0, msglen);
      msg1 = "";
      msg2 = "";
      break;
    case 2:
      msg0 = ms.substring(0, 21);
      msg1 = ms.substring(21, msglen);
      msg2 = "";
      break;
    case 3:
      msg0 = ms.substring(0, 21);
      msg1 = ms.substring(21, 42);
      msg2 = ms.substring(42, msglen);
      break;
  }
}
