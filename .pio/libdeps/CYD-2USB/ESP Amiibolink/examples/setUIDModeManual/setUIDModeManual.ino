/**************************************************************************/
/*!
    @file     setUIDModeManual.ino
    @author   Rennan Cockles
*/
/**************************************************************************/
#include <amiibolink.h>


Amiibolink amiibolink = Amiibolink();

void setup(void) {
  Serial.begin(115200);

  Serial.println("Turn on Amiibolink device");
  delay(1000);

  while (!amiibolink.searchDevice() || !amiibolink.connectToDevice()) {
    Serial.println("Amiibolink device not found. Is it on?");
    delay(500);
  }

  amiibolink.cmdSetUIDModeManual();
}


void loop(void) {}

