#include "../../include/interface.h"
#include <Arduino.h>

// Externs for button states from globals.h, needed by InputHandler
extern volatile bool PrevPress;
extern volatile bool NextPress;
extern volatile bool UpPress;
extern volatile bool DownPress;
extern volatile bool SelPress;
extern volatile bool AnyKeyPress;
extern volatile bool EscPress;

// ===== Runtime device detection flags =====
// These are set once during _setup_gpio() and never change after.
static bool joystickDetected = false;
static bool joystickButtonDetected = false;

/***************************************************************************************
** Function name: _setup_gpio()
** Location: main.cpp
** Description:   initial setup for the device
***************************************************************************************/
void _setup_gpio() {
  // Initial basic pin setup for S3 boards
#ifdef RGB_LED
  if (RGB_LED >= 0) {
    pinMode(RGB_LED, OUTPUT);
    digitalWrite(RGB_LED, LOW);
  }
#endif
#ifdef LED
  if (LED >= 0) {
    pinMode(LED, OUTPUT);
    digitalWrite(LED, LOW);
  }
#endif

  // Setup Joystick Button
#ifdef JOY_BTN_PIN
  if (JOY_BTN_PIN >= 0) {
    pinMode(JOY_BTN_PIN, INPUT_PULLUP);
  }
#endif

  // Detect joystick axes and button
  bool joyX = false, joyY = false, joyBtn = false;
#ifdef JOY_X_PIN
  if (JOY_X_PIN >= 0) {
    int avg = 0;
    for (int i = 0; i < 10; i++) {
      avg += analogRead(JOY_X_PIN);
      delay(1);
    }
    avg /= 10;
    if (avg > 100 && avg < 4000)
      joyX = true; // Valid range for joystick
  }
#endif
#ifdef JOY_Y_PIN
  if (JOY_Y_PIN >= 0) {
    int avg = 0;
    for (int i = 0; i < 10; i++) {
      avg += analogRead(JOY_Y_PIN);
      delay(1);
    }
    avg /= 10;
    if (avg > 100 && avg < 4000)
      joyY = true; // Valid range for joystick
  }
#endif
#ifdef JOY_BTN_PIN
  if (JOY_BTN_PIN >= 0) {
    // Button detection: read multiple times to check if it's stable high (not
    // connected would float)
    int highCount = 0;
    for (int i = 0; i < 10; i++) {
      if (digitalRead(JOY_BTN_PIN) == HIGH)
        highCount++;
      delay(1);
    }
    if (highCount >= 8)
      joyBtn = true; // Mostly high, likely connected with pullup
  }
#endif

  // Only enable joystick if BOTH axes are detected
  joystickDetected = joyX && joyY;
  joystickButtonDetected = joyBtn;
  if (!joystickDetected && (joyX || joyY)) {
    Serial.println("Warning: Joystick partially detected. Check connections.");
  }

  // CC1101 GDO Pins
#ifdef CC1101_GDO0
  if (CC1101_GDO0 >= 0)
    pinMode(CC1101_GDO0, INPUT);
#endif
#ifdef CC1101_GDO2
  if (CC1101_GDO2 >= 0)
    pinMode(CC1101_GDO2, INPUT);
#endif

  // NRF24 CE Pin
#ifdef NRF24_CE_PIN
  if (NRF24_CE_PIN >= 0)
    pinMode(NRF24_CE_PIN, OUTPUT);
#endif

  // startupApp is user-configurable, do not force here
}

/***************************************************************************************
** Function name: getBattery()
** location: display.cpp
** Description:   Delivers the battery value from 1-100
***************************************************************************************/
int getBattery() { return 0; } // S3 board has no battery monitoring by default

/***************************************************************************************
** Function name: isCharging()
** Description:   Default implementation that returns false
***************************************************************************************/
bool isCharging() { return false; }

/*********************************************************************
** Function: setBrightness
** location: settings.cpp
** set brightness value
**********************************************************************/
void _setBrightness(uint8_t brightval) {
#ifdef TFT_BL
  if (TFT_BL >= 0) {
    analogWrite(TFT_BL, brightval);
  }
#endif
}

/*********************************************************************
** Function: InputHandler
** Handles the variables PrevPress, NextPress, SelPress, AnyKeyPress and
* EscPress
**********************************************************************/
void InputHandler(void) {
  static uint32_t last_check = 0;
  if (millis() - last_check < 20)
    return;
  last_check = millis();

  // Reset flags
  static bool btn_last_state = HIGH;

#ifdef JOY_BTN_PIN
  if (JOY_BTN_PIN >= 0) {
    bool btn_state = digitalRead(JOY_BTN_PIN);
    if (btn_state == LOW && btn_last_state == HIGH) {
      SelPress = true;
      AnyKeyPress = true;
    }
    btn_last_state = btn_state;
  }
#endif

  // Joystick Navigation - KY-023 Funduino Shield Configuration
  // Valores calibrados para KY-023: ponto central ~2048, faixa de ativação
  // ~1500-3500
#ifdef JOY_X_PIN
  if (JOY_X_PIN >= 0) {
    int x = analogRead(JOY_X_PIN);
    // Centro: ~2048, Limite inferior: < 1500 (esquerda), Limite superior: >
    // 3500 (direita)
    if (x < 1500) {
      PrevPress = true;
      AnyKeyPress = true;
    } else if (x > 3500) {
      NextPress = true;
      AnyKeyPress = true;
    }
  }
#endif

#ifdef JOY_Y_PIN
  if (JOY_Y_PIN >= 0) {
    int y = analogRead(JOY_Y_PIN);
    // Centro: ~2048, Limite inferior: < 1500 (cima), Limite superior: > 3500
    // (baixo)
    if (y < 1500) {
      UpPress = true;
      AnyKeyPress = true;
    } else if (y > 3500) {
      DownPress = true;
      AnyKeyPress = true;
    }
  }
#endif
}

// ===== Getter functions for joystick detection =====
bool getJoystickDetected() { return joystickDetected; }

bool getJoystickButtonDetected() { return joystickButtonDetected; }
