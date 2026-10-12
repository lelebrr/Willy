/**************************************************************************/
/*!
    @file     readLF.ino
    @author   Rennan Cockles
*/
/**************************************************************************/
#include <chameleonUltra.h>


ChameleonUltra chmUltra = ChameleonUltra();

void setup(void) {
  Serial.begin(115200);

  Serial.println("Turn on Chameleon device");
  delay(1000);

  while (!chmUltra.searchChameleonDevice() || !chmUltra.connectToChamelon()) {
    Serial.println("Chameleon device not found. Is it on?");
    delay(500);
  }

  Serial.println("Waiting for a Low Frequency Card ...");
}


void loop(void) {
  if (!chmUltra.cmdLFRead()) return;

  Serial.print("UID: ");
  for (byte i = 0; i < chmUltra.lfTagData.size; i++) {
    Serial.print(chmUltra.lfTagData.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(chmUltra.lfTagData.uidByte[i], HEX);
  }
  Serial.println();
}

