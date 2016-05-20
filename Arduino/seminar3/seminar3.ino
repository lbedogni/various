#include <SmartphoneWifi.h>

#include "DHT.h"
#include <ESP8266WiFi.h>

const char* ssid     = WIFI_SSID;
const char* password = WIFI_PWD;

#define UPDATES_FREQUENCY 10 * 1000
#define API_KEY SEMINAR

#define DHTPIN 2      
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

IPAddress ip = (0,0,0,0);
IPAddress netmask = (0,0,0,0);
IPAddress gateway = (0,0,0,0);
uint32_t bssid;

void setup() {
  Serial.begin(9600);
  dht.begin();
  delay(2000);
  Serial.println("BEGIN");
}

void loop() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  while (isnan(h) || isnan(t)) {
    delay(1000);
    Serial.println("Failed to read from DHT sensor!");
    h = dht.readHumidity(true);
    t = dht.readTemperature(false,true);
  }

  float hic = dht.computeHeatIndex(t, h, false);
  Serial.println("*************");
  Serial.print("Humidity = ");
  Serial.println(h);
  Serial.print("Temperature = ");
  Serial.println(t);
  Serial.print("Heat Index = ");
  Serial.println(hic);
  Serial.println("---------------");

  unsigned long time = millis();
  WiFi.mode(WIFI_STA);
  Serial.println(String(millis() - time));
  if (ip != (0,0,0,0) && gateway != (0,0,0,0) && netmask != (0,0,0,0)) {
    WiFi.begin(ssid, password, 11);
    WiFi.config(ip, gateway, netmask);
    Serial.println("Configuring..");
    delay(10);
  } else {
    WiFi.begin(ssid, password);
  }
  WiFi.waitForConnectResult();
  ip = WiFi.localIP();
  netmask = WiFi.subnetMask();
  gateway = WiFi.gatewayIP();

  WiFiClient client;

   if (client.connect("api.thingspeak.com",80)) {
    unsigned long time2 = millis();
    String postStr = API_KEY;
    postStr +="&field1=";
    postStr += String(t);
    postStr +="&field2=";
    postStr += String(h);
    postStr += "&field3=";
    postStr += String(hic);
    postStr += "&field4=";
    postStr += String(time2 - time);
    postStr += "\r\n\r\n";

    Serial.println(postStr);

    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + String(API_KEY) + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);

    Serial.println("Took: " + String(time2 - time));
   }
  Serial.println(String(millis() - time));
  WiFi.disconnect();

  delay(UPDATES_FREQUENCY);
}
