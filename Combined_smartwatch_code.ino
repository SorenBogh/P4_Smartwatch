/*
Code currently has the functionality:
  Tells time
  Tells temperature
  Tells watch orientation
  Tells weatherdata
  LDR
  Heartrate monitor 

Things yet to be implemented:
  -SMS'er/Messenger - Thoaaar has the code
  -consistent communication with display

Things to do when all is implemented:
  -Set priories based on all of the tasks instead of only the ones they were developed alongside
  -set vTaskDelay to a reasonable value
  -ensure that there is full utilization of the dual core processor and possibly the ULP
  -Make sure all serial communication with the display is understood correctly by the pixxi28 processor.
  -Make sure the use of freeRTOS does not have unforseen consequences.
*/

/*  
Introduction
  
  Her vil der være kode delen for WiFi blocken af P4 projectet.

  De følgende librarys bruges:
    HTTPClient af Arduino, ellers kan den findes her https://github.com/amcewen/HttpClient
    ArduinoJson af Benoit Blanchon

    Udover librarys skal der også indsættes følgende link i file -> preferences -> additional boards manager URL's:
    https://dl.espressif.com/dl/package_esp32_index.json
    Og der skal downloades ESP32 af Espressif Systems, fra board manager.

*/

// PulseSensor values used to save and calculate BPM
  int pscurrentVal = 0;
  int pspreviousVal = 0;
  int psmaxVal = 0;
  int psminVal = 0;
  int pscheckVal = 0;
  float psdiff = 0;
  bool psonce = false;
  float Comp[9];
  float psmaxval = 0;  //max værdi i forhold til diff
  float psminval = 0;  //min værdi i arrayet til bestemmelse af diff
  int psBPM=0; // beatsPerMinute
  int pscurrentBeat; 
  int pslastBeat;

//test bools for weather functionality
  bool Testing = false;      //Prints the API string
  bool DataToSerial = true;  //Prints the Weather data after assigning

//setup of multicore for FreeRTOS
  
  static const BaseType_t pro_cpu = 0;
  static const BaseType_t app_cpu = 1;

//Libraries
  #include <WiFi.h>
  #include <HTTPClient.h>
  #include <ArduinoJson.h>
  #include <time.h>
  #include <HardwareSerial.h>  // allow us to set which pins we want to use for serial communication, which is needed to communicate with the display
  #include <Wire.h>            // used for I2C communication between esp32 and sensors

// pins for display communication
  #define RESETLINE 33  // pin 33 is used to reset the display
  #define RXp2 25       // Sets the TX and RX pins to pin 25 and 26. This is necessary as the standard pins for an esp32 are used for the built in display on the ttgo t1 board
  #define TXp2 26



// Constants used in the watch orientation program and temperature
  const uint8_t sda = 21; // used for I2C communication
  const uint8_t scl = 22;
  int16_t AccXdata, AccYdata, AccZdata; // accelerometer data
  float AccX, AccY, AccZ;
  int16_t tempData;
  float Roll, Pitch, Temp;
  float r2D = (3.14 / 180);  //Radian to Degrees


//WiFi setup
  char ssid[] = "OnePlus 8T"; // name of wifi
  char password[] = "humahe47"; // wifi password
  IPAddress ip;
//API setup
  HTTPClient client;

//API key
  String OpenWeatherKey = "d5fe945d4fb1c980e0a1ced33b22945f";

//setting location
  String City = "Aalborg";
  String Country = "DK";

//Payload
  String payload = "";


//Document for data parsing
  StaticJsonDocument<1024> doc;

//Timer setup for weatherdata
  unsigned long lasttime;
  unsigned long timerdelay = 60000;

//WeatherData setup
  struct WeatherData {
//geographical position
  float coord_lat;
  float coord_lon;

//Weather description. If time can use to show an icon
  const char* weather_main;
  const char* weather_description;

//Main array of temperature and humidity
  float temp;
  float temp_feels_like;
  float temp_min;
  float temp_max;
  int8_t humidity;

//Wind data
  float wind_speed;
  int16_t wind_deg;
  const char* wind_direction;  // note: Dette skal lige fjernes, hvis det ikke fungere. Så skal det nemlig være string.

//Cloudy
  int8_t clouds_all;  //How clouded is it? Given in percentage.

  //Time of request
  long dt;

  //System and sunset
  const char* sys_country;
  long sys_sunrise;
  long sys_sunset;

  const char* sys_name;
  };

WeatherData WeatherNow;  //A Global struct "case"

// LDR constants
  int LDR; // this is the integer where the data is saved
  const int LDRsensor = 12; // Pin 12 is the pin used for LDR measurement
  const int res = 12; // 12 is the resolution of the ADC used



// Used for saving the time, and later sending it to the display
  static uint8_t conv2d(const char* p) {
  uint8_t v = 0;
  if ('0' <= *p && *p <= '9')
    v = *p - '0';
  return 10 * v + *++p - '0';
  }
uint8_t hh = conv2d(__TIME__), mm = conv2d(__TIME__ + 3), ss = conv2d(__TIME__ + 6); // saves compile time to hh, mm and ss, meaning hours, minutes and seconds.

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, RXp2, TXp2);  // sets the earlier declared pins to be our serial2 tx/rx pins and starts communication.
  pinMode(36, INPUT); // used as the pin for the pulseSensor
  WiFiConnect(ssid, password, 50000); // connects Esp32 to the wifi it has been assigned if available
  pinMode(RESETLINE, OUTPUT);  // allows us to reset the display
  digitalWrite(RESETLINE, 0);  // Reset the Display via pin 33
  vTaskDelay(100 / portTICK_PERIOD_MS);
  digitalWrite(RESETLINE, 1);  // unReset the Display via pin 33
  vTaskDelay(3500 / portTICK_PERIOD_MS);
  
  // prints time to display
    Serial2.print("a");
    Serial2.print(hh);  // prints hours to display
    Serial2.print(mm);  // prints minutes to display
    Serial2.print(ss);  // prints seconds to display

  //Choosing accelerometer' range
    Wire.beginTransmission(0x68);
    Wire.write(0x1C);
    Wire.write(0x0);
    Wire.endTransmission();

  // making the functions into tasks using FreeRTOS
    xTaskCreatePinnedToCore(WeatherDataNow, "Weather", 3500, NULL, 3, NULL, app_cpu);
    xTaskCreatePinnedToCore(internalTemp, "Temp", 1000, NULL, 4, NULL, tskNO_AFFINITY);
    xTaskCreatePinnedToCore(watchOrientation, "Orientation", 1000, NULL, 5, NULL, tskNO_AFFINITY);
    xTaskCreatePinnedToCore(Brightness, "Brightness Adjust", 3500, NULL, 6, NULL, tskNO_AFFINITY);
    xTaskCreatePinnedToCore(pulseSensor,"BPM", 3500, NULL, 7, NULL, tskNO_AFFINITY);
}

void loop() {}

// weatherdata task
void WeatherDataNow(void* pvParameters) {
  while (1) {
    if (WiFi.status() == WL_CONNECTED) {

      WeatherNow = getData();

    } else {
      Serial.println("Lost connection to WiFi");
      WiFiConnect(ssid, password, 10000);
    }

    vTaskDelay(timerdelay / portTICK_PERIOD_MS);
    
  }
}

//WiFi functions for weatherdata
void WiFiConnect(char SSID[], char pass[], int WiFiDelay) {

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network:
    WiFi.begin(ssid, pass);
    vTaskDelay(WiFiDelay / portTICK_PERIOD_MS);
  }

  Serial.println("\nYou're connected to the network!");
  //Print the local IP adress
  Serial.print("Local IP adress: ");
  ip = WiFi.localIP();
  Serial.println(ip);
  printWifiData();
}

// prints wifi stuff for weatherdata
void printWifiData() {
  Serial.print("Connected SSID: ");
  Serial.println(WiFi.SSID());

  //WiFi connection strength
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI):");
  Serial.println(rssi);
}

//API functions used for weatherdata
String APICall(String CityDef, String CountryDef, String OpenWeatherKeyDef) {

  //Makeing the API call to OpenWeatherMaps
  client.begin("http://api.openweathermap.org/data/2.5/weather?q=" + CityDef + "," + CountryDef + "&APPID=" + OpenWeatherKeyDef + "&units=metric");
  int ClientStatus = client.GET();

  //Checking if an error has occured in the API call / client connection
  if (ClientStatus > 0) {
    payload = client.getString();

    if (Testing == true) {
      Serial.print("\n");
      Serial.print("Status:");
      Serial.println(String(ClientStatus));
      Serial.println(payload);
      Serial.print("\n");
    }
  } else {
    Serial.println("Error in HTTP request");
    Serial.println(ClientStatus);
  }
  client.end();
  return payload;
}

// Data processing & Deserialization
WeatherData getData() {

  payload = APICall(City, Country, OpenWeatherKey);

  WeatherData get;

  //The JSON data is deserailized into an array, which is usable in C programming language
  char input[1024];  // Has to be the same size as the document
  payload.toCharArray(input, 1024);

  DeserializationError err = deserializeJson(doc, input);

  //Checking for error during deserialization
  switch (err.code()) {
    case DeserializationError::Ok:
      Serial.println(F("Deserialization succeeded"));
      break;
    case DeserializationError::InvalidInput:
      Serial.println(F("Invalid input!"));
      break;
    case DeserializationError::NoMemory:
      Serial.println(F("Not enough memory"));
      break;
    default:
      Serial.println(F("Deserialization failed"));
      break;
  }

  //Assigning values to data
  get.coord_lat = doc["coord"]["lat"];
  get.coord_lon = doc["coord"]["lon"];

  JsonObject weather = doc["weather"][0];
  get.weather_main = weather["main"];
  get.weather_description = weather["description"];

  JsonObject main = doc["main"];
  get.temp = main["temp"];
  get.temp_feels_like = main["feels_like"];
  get.temp_min = main["temp_min"];
  get.temp_max = main["temp_max"];
  get.humidity = main["humidity"];

  JsonObject wind = doc["wind"];
  get.wind_speed = wind["speed"];
  get.wind_deg = wind["deg"];

  switch (get.wind_deg) {
    case 0 ... 23:
      get.wind_direction = "N";
      break;
    case 24 ... 68:
      get.wind_direction = "NE";
      break;
    case 69 ... 113:
      get.wind_direction = "E";
      break;
    case 114 ... 158:
      get.wind_direction = "SE";
      break;
    case 159 ... 203:
      get.wind_direction = "S";
      break;
    case 204 ... 248:
      get.wind_direction = "SW";
      break;
    case 249 ... 293:
      get.wind_direction = "W";
      break;
    case 294 ... 338:
      get.wind_direction = "NW";
      break;
    case 339 ... 360:
      get.wind_direction = "N";
      break;
  }


  get.clouds_all = doc["clouds"]["all"];

  get.dt = doc["dt"];

  JsonObject sys = doc["sys"];
  get.sys_country = sys["country"];
  get.sys_sunrise = sys["sunrise"];
  get.sys_sunset = sys["sunset"];

  get.sys_name = doc["name"];

  //Printing data, to ensure it is saved right. Automatically sends the data to the display
  if (DataToSerial == true) {
    Serial.print("\n\n\n***************** WeatherData ******************\n");
    Serial.print("--Location--\n");
    Serial.printf("Lat: %.2f\n", get.coord_lat);
    Serial.printf("Lon: %.2f\n", get.coord_lon);

    Serial.printf("--Weather disc--\n");
    Serial.printf("Main: %s", get.weather_main);
    Serial.printf("Disc: %s", get.weather_description);
    Serial2.printf("f %s", get.weather_description);

    Serial.printf("--Temperature--\n");
    Serial.printf("Temperature is %.2f c. From %.2f c down to %.2f c\n", get.temp, get.temp_max, get.temp_min);
    Serial2.printf("c %.2f , %.2f , %.2f \n",get.temp, get.temp_max, get.temp_min); // trasnmits to screen three temperatures
    Serial.printf("Due to a humidity of %i, it feels like %.2f c\n", get.humidity, get.temp_feels_like);
    Serial2.printf("d %i , %.2f \n", get.humidity, get.temp_feels_like); // transmits feel like and humidity


    Serial.printf("--Winds--\n");
    Serial.printf("We have wind speeds of %.2f, comming form %s at a degree of %i\n", get.wind_speed, get.wind_direction, get.wind_deg);
    Serial2.printf("e %.2f, %s , %i\n", get.wind_speed, get.wind_direction, get.wind_deg); // transmits wind speed, wind direction and wind direction

    Serial.printf("--Sunrise/set--\n");  //Time is not converted to a right date.
    Serial.print("The sun rises at a time of ");
    TimeConvert(get.sys_sunrise);
    Serial.printf("The sun sets at a time of ");
    TimeConvert(get.sys_sunset);

    Serial.printf("\nThe data is taken from %s, %s\n", get.sys_name, get.sys_country);
    Serial.print("At the time: ");
    TimeConvert(get.dt);
  }
  return get;
}

// Used in weatherdata to get sunrize and sundown
void TimeConvert(long timegiven) {
  /*
    This code has been made to only account for the time zone of CET. And will therefor not work in other time zones.
  */
  time_t rawtime = timegiven;
  struct tm ts;
  char buf[80];

  // Formatting time to "ddd yyyy-mm-dd hh:mm:ss zzz" like "Thu 2023-05-04 10:29:40 CET"
  ts = *gmtime(&rawtime);
  ts.tm_hour += 2;
  strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S CET", &ts);
  Serial.printf("%s\n", buf);
  Serial2.printf("g %s\n", buf);
}

// task measures the orientation of the watch based on an accelerometer
void watchOrientation(void* parameters) {
 while (1) {
   
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
    Serial.println("Turning on Screen O");
    Serial2.println("b , 15");
  }
  else {
    Serial.println("Turning off Screen O");
    Serial2.println("b , 0");
  }
  vTaskDelay(1000 / portTICK_PERIOD_MS);
 }
}

// task used for measuring the temperature at the sensor. Ideally place near the battery
void internalTemp(void* parameters) {
  while (1) {  
  //Getting measurements from Temperature Sensor
  Wire.beginTransmission(0x68);
  Wire.write(0x41);
  Wire.endTransmission();
  Wire.requestFrom(0x68, 2);
  tempData = Wire.read() << 8 | Wire.read();
  Temp = (float)tempData;
  Temp = (Temp / 340) + 36.53;

    if (Temp > 80) {
      Serial.println("Turning Off the Watch. Watch is too hot");
      Serial2.println("b , 0");
      vTaskDelay(3000 / portTICK_PERIOD_MS);
    } 
    else {
      Serial.println(" Watch not burning");
      vTaskDelay(3000 / portTICK_PERIOD_MS);
    }
  }
}
 
// Used in pulseSensor
void finddiff() {
  for (int i = 0; i < 10; i++) {
    Comp[i] = analogRead(36);
    vTaskDelay (75 / portTICK_PERIOD_MS);
  }

  psmaxval = Comp[0];
  psminval = Comp[0];

  for (int i = 0; i < (10 / sizeof(Comp[0])); i++) {
    psmaxval = max(Comp[i], psmaxval);
    psminval = min(Comp[i], psminval);
  }
  psdiff = psmaxval - psminval;
}

// pulseSensor task
void pulseSensor(void* parameters){
 while (1){
  // put your main code here, to run repeatedly:
  millis();
  finddiff();
  pspreviousVal = pscurrentVal;
  pscurrentVal = analogRead(36);

  if (pscurrentVal > psmaxVal) {
    psmaxVal = pscurrentVal;
  }

  if (pscurrentVal < psminVal) {
    psminVal = pscurrentVal;
  }

  pscheckVal = (psmaxVal - psminVal);
  Serial.print("current: ");
  Serial.print(pscurrentVal);
  Serial2.print("h");
  Serial2.print(pscurrentVal);
  Serial2.println(pscheckVal);
  Serial.print("    check: ");
  Serial.println(pscheckVal);

  if (pscurrentVal >= pscheckVal && psonce == false) {
    psonce = true;
    pslastBeat=pscurrentBeat;
    pscurrentBeat=millis();
   psBPM = ((pscurrentBeat-pslastBeat)*60)/1000;
    if ((psBPM>45) && (psBPM<110)) {
    //Serial.println(BPM);
    }    

  } else if (pscurrentVal <= pscheckVal - 10) {
    psonce = true;
  }
 vTaskDelay (1000 / portTICK_PERIOD_MS);
 }
}

//LDR based screen brightness control task
void Brightness (void *parameters) {
  while (1) {
  analogReadResolution(res);
  LDR = analogRead(LDRsensor);

    if (LDR <= 1023) {
        Serial2.println("b , 3");
        }
        else if ((LDR > 1023) && (LDR <=2047)) {
        Serial2.println("b , 7");
        }
        else if ((LDR > 2047) && (LDR <=3071)) {
        Serial2.println("b , 11");
        }
        else {
        Serial2.println("b , 15");
        }  
    }
    vTaskDelay (1000 / portTICK_PERIOD_MS);
  }



