#include "DHT.h"
#include <WiFiClient.h>
#include <ESP8266WiFi.h>
#include "SmartphoneWifi.h"

//const char* ssid     = WIFI_SSID;
//const char* password = WIFI_PWD;
const char* ssid     = "Wilma-Lab";
const char* password = "wilmawifi!";

boolean amIMaster;
boolean apOn;

#define DHTPIN 2      
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

WiFiServer server(80);

// Arrays
String dataArray[10];

void setup() {
  amIMaster = false;
  apOn = false;
  dht.begin();
  Serial.begin(9600);
  delay(1000);
}

void setupWiFi() {
  WiFi.mode(WIFI_AP);
  uint8_t mac[WL_MAC_ADDR_LENGTH];
  WiFi.softAPmacAddress(mac);
  WiFi.softAP("MOBILESYSTEMS", "thisisthepassword");
  apOn = true;
}

float h;
float t;
float hic;

void loop() {
  if (amIMaster) {
    if (!apOn) {
      Serial.println("Switching the AP on");
      setupWiFi();
      WiFi.softAPIP().printTo(Serial);
      server.begin();
    }
    // Check if a client has connected
    WiFiClient client = server.available();
    if (!client) {
      return;
      delay(10);
    }

    // Check if it is posting something
    String req = client.readStringUntil('\r');
    Serial.println(req);
    if (req.indexOf("posting/") != -1) {
      int firstBar = req.indexOf("/") + 1;
      int secondBar = req.indexOf("/",firstBar) + 1;
      int ipAddr = req.substring(firstBar, secondBar).toInt();
      dataArray[ipAddr] = req.substring(secondBar);
//      otherData = "* - " + String(req.substring(req.indexOf("/") + 1));
    }
    
    h = dht.readHumidity();
    t = dht.readTemperature();

    while (isnan(h) || isnan(t)) {
      delay(1000);
      Serial.println("Failed to read from DHT sensor!");
      h = dht.readHumidity(true);
      t = dht.readTemperature(false,true);
    }

    hic = dht.computeHeatIndex(t, h, false);
    String s = "HTTP/1.1 200 OK\r\n";
    s += "Content-Type: text/html\r\n\r\n";
    s += "<!DOCTYPE HTML>\r\n<html>\r\n";

    s += "Temperature: ";
    s += String(t);
    s += ", Humidity: ";
    s += String(h);
    s += "<br>";
    for (int j = 2; j < 10; j++) {
      if (dataArray[j] != "") {
        s += "192.168.4.";
        s += String(j);
        s += " - ";
        s += dataArray[j];
        s += "<br>";
      }
    }
    
    s += "</html>\n";

    // Send the response to the client
    client.print(s);
    delay(1);
    Serial.println("Client disconnected");
  } else {
    Serial.println("I have to provide my sensed data");
    Serial.println("... but first I have to check whether there is another AP!");
    int n = WiFi.scanNetworks();
    boolean found = false;
    for (int i = 0; i < n; ++i) {
      if (WiFi.SSID(i) == "MOBILESYSTEMS") {
        found = true;
      }
    }
    if (found) {
      // We found the network, so just connect to it and send the data
      Serial.println("We found another network");
      apOn = false;
      amIMaster = false;
      
      h = dht.readHumidity();
      t = dht.readTemperature();
      while (isnan(h) || isnan(t)) {
        delay(1000);
        Serial.println("Failed to read from DHT sensor!");
        h = dht.readHumidity(true);
        t = dht.readTemperature(false,true);
      }
      hic = dht.computeHeatIndex(t, h, false);

      Serial.println("Sending data...");
      WiFi.mode(WIFI_STA);
      WiFi.begin("MOBILESYSTEMS", "thisisthepassword");
      WiFi.waitForConnectResult();
      WiFiClient client;
      client.connect("192.168.4.1",80);
      String myIP = toCharArray(client.localIP());
      client.print("GET posting/" + myIP +"/Temperature: " + String(t) + ", Humidity: " + String(h));
      Serial.println("Data sent");
    } else {
      Serial.println("No networks found. Looks like I'm the master now!");
      amIMaster = true;
    }
  }
  Serial.println("");
  delay(5000);
}

String toCharArray(IPAddress ip) {
  return String(ip[3]); 
}
