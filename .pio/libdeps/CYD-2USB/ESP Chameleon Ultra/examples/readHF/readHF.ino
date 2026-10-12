/**************************************************************************/
/*!
    @file     readHF.ino
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

  Serial.println("Waiting for a High Frequency Card ...");
}


void loop(void) {
  if (!chmUltra.cmd14aScan()) return;

  Serial.print("Card Type: ");
  Serial.println(chmUltra.getTagTypeStr(chmUltra.hfTagData.sak));

  Serial.print("UID: ");
  for (byte i = 0; i < chmUltra.hfTagData.size; i++) {
    Serial.print(chmUltra.hfTagData.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(chmUltra.hfTagData.uidByte[i], HEX);
  }
  Serial.println();

  Serial.print("SAK: ");
  Serial.print(chmUltra.hfTagData.sak < 0x10 ? "0" : "");
  Serial.print(chmUltra.hfTagData.sak, HEX);
  Serial.println();

  Serial.print("ATQA: ");
  for (byte i = 0; i < 2; i++) {
    Serial.print(chmUltra.hfTagData.atqaByte[i] < 0x10 ? " 0" : " ");
    Serial.print(chmUltra.hfTagData.atqaByte[i], HEX);
  }
  Serial.println();
}

