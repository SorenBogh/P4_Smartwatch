/*  Introduction
  
  Her vil der være kode delen for WiFi blocken af P4 projectet.

  De følgende librarys bruges:
    HTTPClient af Arduino, ellers kan den findes her https://github.com/amcewen/HttpClient
    ArduinoJson af Benoit Blanchon

    Udover librarys skal der også indsættes følgende link i file -> preferences -> additional boards manager URL's:
    https://dl.espressif.com/dl/package_esp32_index.json
    Og der skal downloades ESP32 af Espressif Systems, fra board manager.

*/

//test bools
  bool Testing = false;             //Prints the API string
  bool DataToSerial = true;        //Prints the Weather data after assigning

//setup of multicore for FreeRTOS
  #if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
  #else
  static const BaseType_t app_cpu = 1;
  #endif

//Libraries
  #include <WiFi.h>                               
  #include <HTTPClient.h>                         
  #include <ArduinoJson.h>  
  #include <time.h>
                

//WiFi setup
  /*  
  char ssid[] = "TP-Link_E6F2";
  char password[] = "30952522";
  */
  /* Hotspot test*/
  char ssid[] = "Test_SW";
  char password[] = "SmartWatch";
  

  IPAddress ip;

//API setup

  HTTPClient client;

  //API key
  String OpenWeatherKey = "d5fe945d4fb1c980e0a1ced33b22945f";

  //setting location
  String City = "Aalborg";
  String Country = "DK";

  //Payload
  String payload  = "";


//Document for data parsing
  StaticJsonDocument <1024> doc;      

//Timer setup
  unsigned long lasttime;
	unsigned long timerdelay = 5000;

//WeatherData setup
  struct WeatherData{
    //posistion
    float coord_lat;
    float coord_lon;

    //Weather description. If time can use to show an icon
    const char* weather_main;
    const char* weather_description;

    //Main array / temp and humid
    float temp;
    float temp_feels_like;
    float temp_min;
    float temp_max;
    int8_t humidity;

    //Winds
    float wind_speed;
    int16_t wind_deg;
    const char* wind_direction;  // note: Dette skal lige fjernes, hvis det ikke fungere. Så skal det nemlig være string.

    //Cloudy
    int8_t clouds_all; //How clouded is it? Given in percentage.

    //Time of request
    long dt;

    //System and sunset
    const char* sys_country;
    long sys_sunrise; 
    long sys_sunset; 

    const char* sys_name;

  };

  WeatherData WeatherNow;  //A Global struct "case"


void setup() {
  Serial.begin(115200);
  
  WiFiConnect(ssid, password, 10000);
  /*
  xTaskCreate(
              WiFiConnect,      //Pointer to function 
              "WiFiCon",        //Name of task
              200,              //Stack size of task. Ammount in words
              NULL,             //Parameter to pass to function
              1,                //Tast priority
              NULL);            //Task handle
  */
  xTaskCreate(WeatherDataNow, "Weather", 3500, NULL, 2, NULL);

}

void loop() {

}


void WeatherDataNow(void* pvParameters){
  while(1){
    if(WiFi.status() == WL_CONNECTED){

      WeatherNow = getData();

    }
    else{
     Serial.println("Lost connection to WiFi");
     WiFiConnect(ssid, password, 10000);
    }

    vTaskDelay(timerdelay / portTICK_PERIOD_MS);

  }
}


//WiFi functions
  void WiFiConnect(char SSID[], char pass[], int WiFiDelay){

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

  void printWifiData(){
    Serial.print("Connected SSID: ");
    Serial.println(WiFi.SSID());

    //WiFi connection strength
    long rssi = WiFi.RSSI();
    Serial.print("Signal strength (RSSI):");
    Serial.println(rssi);

    // Print the encryption type

    /* Error in getting the encryption type..    Will try to get it working if there is time
    byte encryption = WiFi.encryptionType();
    Serial.print("Encryption Type:");
    Serial.println(encryption, HEX);
    Serial.println();
    */
  }

//API functions

  String APICall(String CityDef, String CountryDef, String OpenWeatherKeyDef){

      //Makeing the API call to OpenWeatherMaps
      client.begin ("http://api.openweathermap.org/data/2.5/weather?q=" + CityDef + "," + CountryDef + "&APPID=" + OpenWeatherKeyDef + "&units=metric");
      int ClientStatus = client.GET();

      //Checking if an error has occured in the API call / client connection
      if(ClientStatus > 0){
        payload = client.getString();

        if(Testing == true){
        Serial.print("\n");
        Serial.print("Status:");
        Serial.println(String(ClientStatus));
        Serial.println(payload);
        Serial.print("\n");
        }
        }else{
        Serial.println("Error in HTTP request");
        Serial.println(ClientStatus);
      }

      client.end();
      return payload;
  }



// Data processing & Deserialization
  WeatherData getData(){

    payload = APICall(City, Country, OpenWeatherKey);

    WeatherData get;

    //The JSON data is deserailized into an array, which is usable in C programming language
      char input[1024];                                         // Has to be the same size as the document
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

        switch(get.wind_deg){
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

    //Printing data, to ensure it is saved right.
      if(DataToSerial == true){
        Serial.print("\n\n\n***************** WeatherData ******************\n");
        Serial.print("--Location--\n");
        Serial.printf("Lat: %.2f\n", get.coord_lat);
        Serial.printf("Lon: %.2f\n", get.coord_lon);

        Serial.printf("--Weather disc--\n");
        Serial.printf("Main: %s", get.weather_main);
        Serial.printf("Disc: %s", get.weather_description);

        Serial.printf("--Temperature--\n");
        Serial.printf("Temperature is %.2f c. From %.2f c down to %.2f c\n", get.temp, get.temp_max, get.temp_min);
        Serial.printf("Due to a humidity of %i, it feels like %.2f c\n", get.humidity, get.temp_feels_like);

        Serial.printf("--Winds--\n");
        Serial.printf("We have wind speeds of %.2f, comming form %s at a degree of %i\n", get.wind_speed, get.wind_direction, get.wind_deg);

        Serial.printf("--Sunrise/set--\n");                                 //Time is not converted to a right date.
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

void TimeConvert(long timegiven){
  /*
    This code has been made to only account for the time zone of CET. And will therefor not work in other time zones.
  */

  time_t rawtime = timegiven;
    struct tm  ts;
    char       buf[80];


    // Formatting time to "ddd yyyy-mm-dd hh:mm:ss zzz" like "Thu 2023-05-04 10:29:40 CET"
    ts = *gmtime(&rawtime);
    ts.tm_hour += 2;
    strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S CET", &ts);
    printf("%s\n", buf);
}