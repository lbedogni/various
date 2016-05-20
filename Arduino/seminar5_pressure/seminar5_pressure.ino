#include <Wire.h>
#include <Adafruit_BMP085.h>
#include <WiFiClient.h>
#include <ESP8266WiFi.h>
#include "SmartphoneWifi.h"

//const char* ssid     = WIFI_SSID;
//const char* password = WIFI_PWD;
const char* ssid     = "Wilma-Lab";
const char* password = "wilmawifi!";

boolean amIMaster;
boolean apOn;

WiFiServer server(80);

int SDA_pin = 2;
int SCL_pin = 0;
Adafruit_BMP085 bmp;

void setup() {
  Serial.begin(9600);
  amIMaster = false;
  apOn = false;
  delay(1000);
  Serial.println("Starting....");
  Wire.begin(SDA_pin, SCL_pin);
  if (!bmp.begin()) {
    Serial.println("Could not find a valid BMP085 sensor, check wiring!");
    while (1) {}
  }
}

void setupWiFi() {
  WiFi.mode(WIFI_AP);
  uint8_t mac[WL_MAC_ADDR_LENGTH];
  WiFi.softAPmacAddress(mac);
  WiFi.softAP("MOBILESYSTEMS", "thisisthepassword");
  apOn = true;
}

float p;
float t;

// Arrays
String dataArray[10];

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
    
    p = bmp.readPressure();
    t = bmp.readTemperature();

    Serial.println("Pressure: " + String(p) + ", Temperature: " + String(t));
    
    String s = "HTTP/1.1 200 OK\r\n";
    s += "Content-Type: text/html\r\n\r\n";
    s += "<!DOCTYPE HTML>\r\n<html>\r\n";

    s += "Temperature: ";
    s += String(t);
    s += ", Pressure: ";
    s += String(p);
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
      
      p = bmp.readPressure();
      t = bmp.readTemperature();
      
      Serial.println("Sending data...");
      WiFi.mode(WIFI_STA);
      WiFi.begin("MOBILESYSTEMS", "thisisthepassword");
      WiFi.waitForConnectResult();
      WiFiClient client;
      client.connect("192.168.4.1",80);
      String myIP = toCharArray(client.localIP());
      client.print("GET posting/" + myIP +"/Temperature: " + String(t) + ", Pressure: " + String(p));
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
