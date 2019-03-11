#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <Adafruit_Sensor.h>
#include "DHT.h"
#include "DHT_U.h"
#include <math.h>
#include <WiFiUdp.h>

#include "secrets.h"

#define ACTIVATE_TWITTER 0
#define INTERVAL 10 * 60 * 1000

//set up temp sensor
#define DHTPIN 4     // what digital pin the DHT22 is conected to
#define DHTTYPE DHT22   // there are multiple kinds of DHT sensors

DHT_Unified dht(DHTPIN, DHTTYPE);

//analog read in for soil moisture sensor
const int AnalogIn  = A0;

//some stuff so that the system knows what time it is
#define NTP_OFFSET   60 * 60 * 2     // In seconds
#define NTP_INTERVAL 60 * 1000    // In miliseconds
#define NTP_ADDRESS  "europe.pool.ntp.org"

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);

//a bunch of constants with paswords and tokens
const char* ssid     = WIFI_SSID;
const char* password = WIFI_PW;

const char* thing_speak_channel_api_key = THINGSPEAK_CHANNEL_API_KEY;
const char* thing_speak_twitter_api_key = THINGSPEAK_TWITTER_API_KEY;

//bunch of variables
int counter = 0;
String tweetText = "";
int randomnes = 20;

//texts to tweet. All arrays are of equal length (5)
int numberOfElemenst = 5;
char* startupTweets[] = {"Hi.+Ich+bin+wach.", "Ja,+Ok.+geht+los.", "Wird+soeben+hochgefahren.+Geduld.+Geduld.", "Dwwwwwwwwwwwwwwwwwww", "And+here+we+go"};
char* soilMoisLow[] = {"Hust+hust.+Schon+recht+trocken+die+Erde...", "Wenn+ichs+nicht+besser+wüsste+würd+ich+denken+das+ist+ne+Wüste.", "So+ne+Gießkanne+könnte+mal.", "So+gieße+er+mich+nun.+Hm...+das+klingt+ekeliger+als+es+gemeint+war.", "Wasser.+Ein+Königreich+für+Wasser."};
char* freshWater[] = {"Boa+Krass+Wasser!+Fett", "Nichts+ist+besser+als+ein+guter+drink.", "Gönn+Dir+Gluck+Gluck.", "Schöne+Sache.+Ich+mein+is+viel+langweilig,+aber+gieß+rein", "Geil+ich+könnt+fast+tauchen+gehen."};
char* hightemp[] = {"It's+getting+hot+in+here+so+photosynthesis.", "HIER+KOMMT+DIE+SONNE!", "So hooooooooooot", "Here+comes+the+sun+dubidubidu+and+temperatures+above+30+degerees.", "Hält ja keiner aus in dieser Hitze..."};
char* tempannounce[] = {"At+the+beep+the+temperature+will+be+", "Temperatur+", "Aktuelle+Temperatur+", "Wie+warm+is+vong+Grad+her?+", "Temp:+"};
char* humidityannounce[] = {"Luftfeuchtigkeit+bei:+", "Luftfeuchtigkeit+bei:+", "Luftfeuchtigkeit+bei:+", "Luftfeuchtigkeit+bei:+", "Luftfeuchtigkeit+bei:+"};

int h2 = -1;
int m2 = -1;
int t2 = -1;

void setup() {
  Serial.begin(115200);
  delay(100);

  // Initialize dht22.
  dht.begin();
  Serial.println(F("DHTxx Unified Sensor Example"));
  // Print temperature sensor details.
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  Serial.println(F("------------------------------------"));
  Serial.println(F("Temperature Sensor"));
  Serial.print  (F("Sensor Type: ")); Serial.println(sensor.name);
  Serial.print  (F("Driver Ver:  ")); Serial.println(sensor.version);
  Serial.print  (F("Unique ID:   ")); Serial.println(sensor.sensor_id);
  Serial.print  (F("Max Value:   ")); Serial.print(sensor.max_value); Serial.println(F("°C"));
  Serial.print  (F("Min Value:   ")); Serial.print(sensor.min_value); Serial.println(F("°C"));
  Serial.print  (F("Resolution:  ")); Serial.print(sensor.resolution); Serial.println(F("°C"));
  Serial.println(F("------------------------------------"));
  // Print humidity sensor details.
  dht.humidity().getSensor(&sensor);
  Serial.println(F("Humidity Sensor"));
  Serial.print  (F("Sensor Type: ")); Serial.println(sensor.name);
  Serial.print  (F("Driver Ver:  ")); Serial.println(sensor.version);
  Serial.print  (F("Unique ID:   ")); Serial.println(sensor.sensor_id);
  Serial.print  (F("Max Value:   ")); Serial.print(sensor.max_value); Serial.println(F("%"));
  Serial.print  (F("Min Value:   ")); Serial.print(sensor.min_value); Serial.println(F("%"));
  Serial.print  (F("Resolution:  ")); Serial.print(sensor.resolution); Serial.println(F("%"));
  Serial.println(F("------------------------------------"));

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

  //we use random to select tweets and need an unconnected pin to seed
  randomSeed(analogRead(1));

  //NTP
  timeClient.begin();
}

void loop() {
  Serial.println(counter);
  timeClient.update();
  tweetText = "";
  //wenn es los geht kommt erstmal ein aufwach tweet
  if (counter == 0) {
    tweetText = startupTweets[random(numberOfElemenst)];
  }

  //code to read in the value
  float m = analogRead(AnalogIn);
  Serial.println(m);

  sensors_event_t event;
  
  float h;
  // Get humidity event and print its value.
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    Serial.println(F("Error reading humidity!"));
  } else {
    Serial.print(F("Humidity: "));
    Serial.print(event.relative_humidity);
    Serial.println(F("%"));
    h = event.relative_humidity;
  }
  
  float t;
  // Get temperature event and print its value.
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println(F("Error reading temperature!"));
  } else {
    Serial.print(F("Temperature: "));
    Serial.print(event.temperature);
    Serial.println(F("°C"));
    t = event.temperature;
  }

  float values[3] = {m, h, t};
  HTTPClient http;

  //tweets about sudden change
  if (m2 != -1 && m2 > 1000 && m < 600) {
    tweetText = freshWater[random(numberOfElemenst)];
  }

  //put in m2...
  m2 = m;
  t2 = t;
  h2 = h;

  //random tweets about the state of things
  int randNumber = random(randomnes);

  if (tweetText == "" && randNumber == 1) {
    //ein tweet über die erdfeuchtigkeit
    if (m > 1000 ) {
// disabled because sensor is out of order
//      tweetText = soilMoisLow[random(numberOfElemenst)];
    }
  }

  if (tweetText == "" && randNumber == 2) {
    tweetText = humidityannounce[random(numberOfElemenst)];
    tweetText.concat(h);
    tweetText.concat("+Prozent");
  }

  if (tweetText == "" && randNumber == 3) {
    //ein tweet über die temperatur
    if(random(5) == 1) {
      tweetText = tempannounce[random(numberOfElemenst)];
      tweetText.concat(t);
      tweetText.concat("+°C");
    }
    if (t > 30 && t < 100 && tweetText == "") { //we check for 100 because sometime sits NaN which is read as 2147483647
      tweetText = hightemp[random(numberOfElemenst)];
    } else {
      
    }
  }

  bool sendData = false;
  String url = "http://api.thingspeak.com/update?api_key=";
  url.concat(thing_speak_channel_api_key);
  // disable moisture
  // if (m < 1024) {
  //  url.concat("&field1=");
  //  url.concat(round(m));
  //  sendData = true;
  //}
  if (h < 100) {
    url.concat("&field2=");
    url.concat(h);
    sendData = true;
  }
  if (t < 100) {
    url.concat("&field3=");
    url.concat(t);
    sendData = true;
  }

  if (sendData == true) {
    Serial.println(url);
    http.begin(url); //HTTP
    int httpCode = http.GET();
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);
      // file found at server
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.println(payload);
      }
    } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  }

  //delay(60000);

  if (tweetText != "" && ACTIVATE_TWITTER > 0) {
    http.begin("http://api.thingspeak.com/apps/thingtweet/1/statuses/update");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    http.addHeader("X-THINGSPEAKAPIKEY", thing_speak_twitter_api_key);

    String body = "api_key=";
    body.concat(thing_speak_twitter_api_key);
    body.concat("&status=");
    body.concat(tweetText);
    body.concat("+(" + timeClient.getFormattedTime() + ")");

    Serial.println(body);

    int httpCode = http.POST(body);

    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] POST... code: %d\n", httpCode);

      // file found at server
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.println(payload);
      }
    } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
  }

  counter++;
  delay(INTERVAL);
}
