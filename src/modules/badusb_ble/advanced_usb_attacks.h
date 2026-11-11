#ifndef __ADVANCED_USB_ATTACKS_H__
#define __ADVANCED_USB_ATTACKS_H__

#include <Arduino.h>

void advancedUsbAtksMenu();

// Attack Functions
void touchMacroRecorder();
void stealthMouseJiggler();
void keyboardMouseWorm();
void conditionalDelayedPayload();
void hidCredentialHarvester();
void ledBlinkExfilChannel();
void multiOsPayloadSwitcher();
void hidTimingSideChannel();
void ransomwareSimulationDemo();
void persistentHidBackdoor();

#endif
