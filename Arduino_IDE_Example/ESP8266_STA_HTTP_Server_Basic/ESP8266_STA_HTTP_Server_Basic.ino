#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

const char* ssid = "ssid";
const char* password = "password";

ESP8266WebServer server(80); // global object "serve" , listening on port 80

void setup() {

  Serial.begin(115200);
  WiFi.begin(ssid, password);
  delay(1000);
  Serial.print("Connecting..");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println(WiFi.localIP());
  server.on("/", handleRootPath); // root request from client triggers handlerootpath
  server.begin();
  Serial.println("Server up");
  
  if (MDNS.begin("esp8266")) {  //Start mDNS

    Serial.println("MDNS started");
    Serial.println("visit http://esp8266.local");

  }

}

void loop() {

 server.handleClient();// looped to handle incoming http traffic
}

void handleRootPath() {
    server.send(200, "text/plain", "Hello world");
 }

