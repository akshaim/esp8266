#include <ESP8266WiFi.h>

const char* ssid = "IoT";
const char* password = "password";
//WiFi.mode(WIFI_STA); // Station (client) by default unless AP enabled 

void setup (){
  WiFi.begin(ssid, password);
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  delay(1000);
  Serial.print("Connecting..");
  while (WiFi.status() != WL_CONNECTED) {
 
    delay(1000);
    Serial.print(".");
 
  }
 
 
}
void loop(){
   Serial.println(WiFi.localIP());
   delay (1000);
   }
