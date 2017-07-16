#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>

const int AnalogIn  = A0;

int readingIn = 0;

const char* ssid     = ""; //Netzwerkname
const char* password = ""; //Passwort

void setup() {
  Serial.begin(115200);
  delay(100);
 
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
 
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  //code to read in the value
  readingIn = analogRead(AnalogIn);
  Serial.println(readingIn);
  
  HTTPClient http;

  String url = "http://api.thingspeak.com/update?api_key=XXXXXXXXXXXXXXXX&field1=";
  url.concat(readingIn);
  
  Serial.println(url);
  
  http.begin(url); //HTTP

  int httpCode = http.GET();

  if(httpCode > 0) {
    // HTTP header has been send and Server response header has been handled
    Serial.printf("[HTTP] GET... code: %d\n", httpCode);

    // file found at server
    if(httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      Serial.println(payload);
    }
   } else {
    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str()); 
   }
        
  http.end();
  
  delay(600000);
}

