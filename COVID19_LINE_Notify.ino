#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif
#include <WiFiClientSecure.h>
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

#include <HTTPClient.h>
#include <WiFi.h>
#include <ArduinoJson.h>

#define MAX_DEVICES 4 // Number of modules connected
#define CLK_PIN   14   // SPI SCK pin on UNO
#define DATA_PIN  12   // SPI MOSI pin on UNO
#define CS_PIN    15   // connected to pin 10 on UNO
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
MD_Parola P = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);
textEffect_t scrollEffect = PA_SCROLL_LEFT;
textPosition_t scrollAlign = PA_LEFT;  // how to aligh the text
int scrollSpeed = 40; 
int scrollPause = 0; 


int cases;
int todayCases;
int deaths;
int todayDeaths;
int recovered;
int newRecovered;
int active;
int critical;
int casesPerOneMillion;
int deathsPerOneMillion;
const char* UdateDate;
#define BUF_SIZE 100
String Cases;
String Line_cases;
char currentMessage[BUF_SIZE] = {""};
bool animateCase = false;
void Printcase();

//Initial Task
void TaskgetJsion( void *pvParameters );
void TaskLEDDotmatrix( void *pvParameters );


//WiFi Configuration
const char* ssid = "your wifi ssid";
const char* password =  "your wifi password";






#define LINE_TOKEN "your LINE Token"

bool LINE_Notify(String message) {
  WiFiClientSecure client;

  if (!client.connect("notify-api.line.me", 443)) {
    Serial.println("connection failed");
    return false;   
  }

  String payload1 = "message=" + message;
  String req = "";
  req += "POST /api/notify HTTP/1.1\r\n";
  req += "Host: notify-api.line.me\r\n";
  req += "Authorization: Bearer " + String(LINE_TOKEN) + "\r\n";
  req += "User-Agent: ESP32\r\n";
  req += "Content-Type: application/x-www-form-urlencoded\r\n";
  req += "Content-Length: " + String(payload1.length()) + "\r\n";
  req += "\r\n";
  req += payload1;
  // Serial.println(req);
  client.print(req);
    
  delay(20);

  // Serial.println("-------------");
  long timeOut = millis() + 30000;
  while(client.connected() && timeOut > millis()) {
    if (client.available()) {
      String str = client.readString();
      // Serial.print(str);
    }
    delay(10);
  }
  // Serial.println("-------------");

  return timeOut > millis();
} 



void setup() {

  P.begin();                                  //Initail MD_PAROLA 
  
  Serial.begin(115200);                       //Initial serial
 
  WiFi.begin(ssid, password);                 //Initial WiFi

  //WiFI Connection Display
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
 
  Serial.println("Connected to the WiFi network");

  //Create task
  xTaskCreatePinnedToCore(TaskgetJsion,  "getJsion",  (1024*32),  NULL,  1,  NULL ,  ARDUINO_RUNNING_CORE);
  xTaskCreatePinnedToCore(TaskLEDDotmatrix,  "LEDDotmatrix",  (1024*32),  NULL,  2,  NULL ,  ARDUINO_RUNNING_CORE);
 
}

void parseJson(const char * jsonString) {
  const size_t capacity = JSON_OBJECT_SIZE(6) + JSON_OBJECT_SIZE(11) + 270;
  DynamicJsonDocument doc(capacity);
  DeserializationError err = deserializeJson(doc, jsonString);
  if (err) {
    Serial.print(F("deserializeJson() failed with code "));
    Serial.println(err.c_str());
  }
  
  Serial.println("parsed");

  
 // const char* country = doc["country"]; // "India"
 // Serial.print("country:  ");
 // Serial.println(country);

  cases = doc["Confirmed"]; // 562
 // Serial.print("cases:  ");
 // Serial.println(cases);
  
  todayCases = doc["NewConfirmed"]; // 26
 // Serial.print("todayCases:  ");
 // Serial.println(todayCases);

  
  deaths = doc["Deaths"]; // 11
 // Serial.print("deaths:  ");
 // Serial.println(deaths);

  
  todayDeaths = doc["NewDeaths"]; // 1
 // Serial.print("todayDeaths:  ");
 // Serial.println(todayDeaths);

  
  recovered = doc["Recovered"]; // 40
 // Serial.print("recovered:  ");
 // Serial.println(recovered);

  newRecovered = doc["NewRecovered"];


  active = doc["Hospitalized"]; // 511
 // Serial.print("active:  ");
 // Serial.println(active);

  
  //critical = doc["critical"]; // 0
 // Serial.print("critical:  ");
 // Serial.println(critical);

  
 // casesPerOneMillion = doc["casesPerOneMillion"]; // 0
 // Serial.print("casesPerOneMillion:  ");
 // Serial.println(casesPerOneMillion);

  
  //deathsPerOneMillion = doc["deathsPerOneMillion"]; // 0
 // Serial.print("deathsPerOneMillion:  ");
 // Serial.println(deathsPerOneMillion);
    UdateDate = doc["UpdateDate"];
}

void loop(void) { 
  
  }
const String endpoint = "https://covid19.th-stat.com/api/open/today";
HTTPClient http;
void TaskgetJsion(void *pvParameters)  // This is a task.
{
  (void) pvParameters;
  int c = 0;
  int d = 0;
  int e = 0;
  int f = 0;
  
 
  for (;;) 
  {
    http.begin(endpoint);
    int httpCode = http.GET();  
    if (httpCode > 0) 
    { 
        String payload = http.getString();
        Serial.println(httpCode);
        Serial.println(payload);
       
        parseJson(payload.c_str());
    }
    else 
    {
      Serial.println("Error on HTTP request");
    }
    http.end();
            Line_cases = "\n Total Cases: "+String(cases)+"\n Today cases: "+String(todayCases)+"\n Deaths: "+String(deaths)+"\n Today Deaths: "+String(todayDeaths)+"\n Recovered: "+String(recovered)+"\n New Recovered: "+String(newRecovered)+"\n Active cases: "+String(active)+ "\n Up to date: "+ (UdateDate); 
        Serial.println(Line_cases);
     if ((cases > c)||(todayCases > d)||(deaths > e)||(recovered > f)) 
     {    
        //Serial.println("Alert !");
        LINE_Notify(Line_cases);
        c = cases; d = todayCases; e = deaths; f = recovered;
        //Serial.println(c);
      }
        else 
          {
          Serial.println("No have update from AIP");
          }
    vTaskDelay(1000000);
  }
}




//Update LED Dotmerix 32x8 text scrolling
void TaskLEDDotmatrix(void *pvParameters)  // This is a task.
{
  (void) pvParameters;

  
  //int g;
  for (;;)
  {
    Printcase();
    if(P.displayAnimate()) 
    {
      if(animateCase) 
      {
        P.displayText(currentMessage, scrollAlign, scrollSpeed, scrollPause, scrollEffect, scrollEffect);
        animateCase = false;
      }
      P.displayReset();  // Reset and display it again
    }   

    vTaskDelay(10);
  }
  
}

void Printcase(){
  Cases= "THA COVID-19 || Cases: "+String(cases)+" || Today cases: "+String(todayCases)+" || Deaths: "+String(deaths)+" || Today Deaths: "+String(todayDeaths)+" || Recovered: "+String(recovered)+" || New Recovered: "+String(newRecovered)+" || Active cases: "+String(active);
  char newMessage[BUF_SIZE];
  Cases.toCharArray(newMessage,BUF_SIZE);

  strcpy(currentMessage,newMessage);
  animateCase = true;
      

  }
