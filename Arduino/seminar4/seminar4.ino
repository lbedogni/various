#include <WiFiClient.h>
#include <ESP8266WiFi.h>

WiFiServer server(80);

void setupWiFi() {
  uint8_t mac[WL_MAC_ADDR_LENGTH];
  WiFi.softAPmacAddress(mac);
  WiFi.softAP("MOBILESYSTEMS", "thisisthepassword");
  //WiFi.softAPConfig(IPAddress(192,168,1,1), IPAddress(192,168,1,1), IPAddress(192,168,1,0));
}

void setup() {
  Serial.begin(9600);
  setupWiFi();
  server.begin();
  delay(1000);
  WiFi.softAPIP().printTo(Serial);
  Serial.println();
}

void loop() {
  WiFiClient client = server.available();
  if (!client) {
    return;
    delay(10);
  }
  Serial.println("Someone connected");
  String s = "HTTP/1.1 200 OK\r\n";
  s += "Content-Type: text/html\r\n\r\n";
  s += "<!DOCTYPE HTML>\r\n<html>\r\n";

  // Send the response to the client
  //client.print(s);
  delay(1);
  Serial.println("Client disconnected");
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0) {
    Serial.println("no networks found");
  } else {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE)?" ":"*");
      delay(10);
      s = s + String(i + 1) + ": " + String(WiFi.SSID(i)) + " (" + String(WiFi.RSSI(i)) + ") - " + String(WiFi.encryptionType(i) == ENC_TYPE_NONE ? " " : "*") + "<br>";
    }
  }
  s += "</html>\n";
  client.print(s);
  client.flush();
  delay(1000);
  delay(1000);
}
