/*

  Created by Akshai M , ICFOSS.

  = SPIFFS
  upload the contents of the data folder with MkSPIFFS Tool ("ESP8266 Sketch Data Upload" in Tools menu in Arduino IDE)
  https://github.com/esp8266/Arduino/blob/master/doc/filesystem.md#uploading-files-to-file-system

  = Javascript GAUGE
  Javascript Gauge
  http://www.esp8266.com/viewtopic.php?p=36089
  http://newmaan.com/gauge.min.js


  Approach derived from https://github.com/roboticboyer/ESP8266_Gauge_AJAX


  Caution  avoid calling all Serial prints else SRAM may overflow.
*/

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "DHT.h"
#include <FS.h>

const char* ssid = "SSID";
const char* password = "password";

ESP8266WebServer server(80);

// DHT
#define DHTPIN D2
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

#define INTERVAL 5
float temp = 20;
float hum = 30;
float webtemp = 20;
float webhum = 30;
long temp_time;
int counter = 0;
int start = 0;
//char str[20];
//------------------------------------------------------------------
String XML;
void buildXML() {
  XML = "<?xml version='1.0'?>"; // start xml response
  XML += "<response>";
  //XML+=random(-10,100);
  XML += "<temperature>";
  XML += webtemp;
  XML += "</temperature>";
  XML += "<humidity>";
  XML += webhum;
  XML += "</humidity>";
  XML += "</response>";// end xml response
}
//------------------------------------------------------------------
void handleXML() {
  buildXML();
  if (start == 0)
  {
    Serial.println("New Client connected");
    delay(500); // delay to avoid scrambling of needle in gauge
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

//content definition for client(browser);
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
//load files from file system
bool handleFileRead(String path) {
  Serial.println("handleFileRead: " + path);
  start = 0; // new client
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
  dht.begin();
  delay(1000);
  Serial.begin(115200);
  Serial.print("\n");
  Serial.setDebugOutput(true);
  //  rtcObject.Begin();
  // RtcDateTime currentTime = RtcDateTime(24, 02, 18, 19, 15, 0); // current date and time
  //  rtcObject.SetDateTime(currentTime);
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
  //WIFI INIT
  Serial.printf("Connecting to %s\n", ssid);
  if (String(WiFi.SSID()) != String(ssid)) {
    WiFi.begin(ssid, password);
  }
  //WiFi.config(IPAddress(192, 168, 1, 220), IPAddress(192, 168, 1, 1), IPAddress(255, 255, 255, 0));

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());

  delay(500);
  if (MDNS.begin("esp8266")) {  //Start mDNS. Webserver can be reached on http://esp8266.local
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
    } while (temp == 85.0 || temp == (-127.0));
    temp_time = millis();

  }

  if (temp != webtemp && isnan(temp) != 1) {
    webtemp = temp;
    Serial.println("Temperature Changed");
    Serial.print("Temperature: ");
    Serial.println(temp);
    counter = 0;
  }
  else if (isnan(temp)) { // condition to avoid nan and detect faulty sensor.
    counter++;
    if (counter > 25){
      webtemp = -10;
    }

  }
  if (hum != webhum && isnan(hum) != 1) {
    webhum = hum;
    Serial.println("Humidity Changed ");
    Serial.print("Humidity: ");
    Serial.println(hum);
  }
}

