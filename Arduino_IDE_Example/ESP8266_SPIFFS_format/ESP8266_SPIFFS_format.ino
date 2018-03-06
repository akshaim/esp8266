
/*
 * Program to format SPIFFS and create a file formatecomplete.
 * If formatcomplete exists the program skips formatting during reset.
*/

#include <FS.h>
void setup() {
  Serial.begin(115200);
  Serial.println();
  SPIFFS.begin();

  if (!SPIFFS.exists("/formatComplete.txt")) {
    Serial.println("Please wait 30 secs for SPIFFS to be formatted");
    SPIFFS.format();
    Serial.println("Spiffs formatted");

    File f = SPIFFS.open("/formatComplete.txt", "w");
    if (!f) {
      Serial.println("file open failed");
    } else {
      f.println("Format Complete");
    }
  } else {
    Serial.println("SPIFFS is formatted. Moving along...");
  }

}

void loop() {
  // put your main code here, to run repeatedly:

}
