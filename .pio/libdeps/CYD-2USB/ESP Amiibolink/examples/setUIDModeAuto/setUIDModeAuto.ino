/**************************************************************************/
/*!
    @file     setUIDModeAuto.ino
    @author   Rennan Cockles
*/
/**************************************************************************/
#include <amiibolink.h>


ChameleonUltra amiibolink = ChameleonUltra();

void setup(void) {
  Serial.begin(115200);

  Serial.println("Turn on Amiibolink device");
  delay(1000);

  while (!amiibolink.searchDevice() || !amiibolink.connectToDevice()) {
    Serial.println("Amiibolink device not found. Is it on?");
    delay(500);
  }

  amiibolink.cmdSetUIDModeAuto();
}


void loop(void) {}

