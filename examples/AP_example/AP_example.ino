#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiAccessPoint.h>

void setup()
{
  // Start serial
  Serial.begin(115200);
  delay(10);
  WiFiConfig("Phong Lan System Care","13571995", true);
}

void loop()
{
  
}
