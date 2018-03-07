/*
  Created by Akshai M , ICFOSS.
  SPIFFS
  upload the contents of the data folder with MkSPIFFS Tool ("ESP8266 Sketch Data Upload" in Tools menu in Arduino IDE)
  https://github.com/esp8266/arduino-esp8266fs-plugin

  Javascript GAUGE
  Javascript Gauge
  http://www.esp8266.com/viewtopic.php?p=36089
  http://newmaan.com/gauge.min.js

  Wifi Manager https://github.com/tzapu/WiFiManager

  Inspiration https://github.com/roboticboyer/ESP8266_Gauge_AJAX

  Caution  avoid calling all Serial prints else SRAM may overflow.


  *****Important**** Reset the module after uploading code.
  Software reset wont work until a manual reset was done in the past hence
  ESP.restart() will fail.

*/


#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h>
#include <ESP8266mDNS.h>
#include "DHT.h"
#include <FS.h>
//#include <EEPROM.h>

ESP8266WebServer server(80);

//DHT
#define DHTPIN D6
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

WiFiManager wifiManager;

const char *ssid = "ServerStatusAP"; // Name of SSID to be created for AP if WiFi manager fails

#define TRIGGER_PIN D3
#define INTERVAL 5 // in sec
float temp = 20;
float hum = 30;
float webtemp = 20;
float webhum = 30;
double temp_time = 0;
int counter = 0;
int start = 0;
int alert = 1;
int wfm = 1;
//-----------------------------------------------------------------
String XML;
void buildXML() {
  XML = "<?xml version='1.0'?>";
  XML += "<response>";
  //XML+=random(-10,100);
  XML += "<temperature>";
  XML += webtemp;
  XML += "</temperature>";
  XML += "<humidity>";
  XML += webhum;
  XML += "</humidity>";
  XML += "<alert>";
  XML += alert;
  XML += "</alert>";
  XML += "</response>";
}
//------------------------------------------------------------------
void handleXML() {
  buildXML();

  if (start == 0) {
    Serial.println("New Client connected");
    delay(500);
    start = 1;
  }
  server.send(200, "text/xml", XML);
  Serial.println("XML Data sent");
}
//----------------------------------------------------------------------
//format bytes
String formatBytes(size_t bytes) {
  if (bytes < 1024) {
    return String(bytes) + "B";
  } else if (bytes < (1024 * 1024)) {
    return String(bytes / 1024.0) + "KB";
  } else if (bytes < (1024 * 1024 * 1024)) {
    return String(bytes / 1024.0 / 1024.0) + "MB";
  } else {
    return String(bytes / 1024.0 / 1024.0 / 1024.0) + "GB";
  }
}
//----------------------------------------------------------------------
String getContentType(String filename) {
  if (server.hasArg("download")) return "application/octet-stream";
  else if (filename.endsWith(".htm")) return "text/html";
  else if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".gif")) return "image/gif";
  else if (filename.endsWith(".jpg")) return "image/jpeg";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".xml")) return "text/xml";
  else if (filename.endsWith(".pdf")) return "application/x-pdf";
  else if (filename.endsWith(".zip")) return "application/x-zip";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}
//----------------------------------------------------------------------
bool handleFileRead(String path) {
  Serial.println("handleFileRead: " + path);
  start = 0;
  if (path.endsWith("/")) path += "index.htm";
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {
    if (SPIFFS.exists(pathWithGz))
      path += ".gz";
    File file = SPIFFS.open(path, "r");
    size_t sent = server.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}
//----------------------------------------------------------------------
void setup(void) {
  pinMode(TRIGGER_PIN, INPUT_PULLUP);
  Serial.begin(115200);
  Serial.print("\n");
  Serial.setDebugOutput(true);
  delay(500);
  wfm = wifiManager.autoConnect("ServerStatusAP");
  if (wfm == 0) {
    delay(1000);
    WiFi.softAP(ssid);
  }
  dht.begin();
  delay(1000);
  SPIFFS.begin();
  {
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      Serial.printf("FS File: %s, size: %s\n", fileName.c_str(), formatBytes(fileSize).c_str());
    }
    Serial.printf("\n");
  }

  if (MDNS.begin("server-status")) {  //Start mDNS

    Serial.println("MDNS started");
  }
  server.on("/xml", handleXML);
  server.onNotFound([]() {
    if (!handleFileRead(server.uri()))
      server.send(404, "text/plain", "FileNotFound");
  });
  server.begin();
  Serial.println("HTTP server started");
}
//----------------------------------------------------------------------
void loop(void) {

  if (WiFi.status() != WL_CONNECTED && wfm == 1 ) {
    ESP.restart();
  }
  if (digitalRead(TRIGGER_PIN) == LOW ) {
    wifiManager.resetSettings();
    ESP.restart();
  }
  server.handleClient();
  yield();
  measure();
}
//----------------------------------------------------------------------
void measure(void) {
  long time = millis();
  if (time > temp_time + INTERVAL) {
    do {
      temp = dht.readTemperature();
      hum = dht.readHumidity();
      //rtc();
    } while (temp == 85.0 || temp == (-127.0));
    temp_time = millis();
  }
  if (temp != webtemp && isnan(temp) != 1) {
    webtemp = temp;
    Serial.println("Temperature Changed");
    Serial.print("Temperature: ");
    Serial.println(temp);
    counter = 0;
    alert = 1;
  }
  else if (isnan(temp) || isnan(hum)) {
    counter++;
    delay(1000);
    if (counter > 5) {
      webtemp = -9;
      webhum = -9;
      alert = 0; // sensor failure detected
    }
  }
  if (hum != webhum && isnan(hum) != 1) {
    webhum = hum;
    Serial.println("Humidity Changed ");
    Serial.print("Humidity: ");
    Serial.println(hum);
    alert = 1;
  }
}

