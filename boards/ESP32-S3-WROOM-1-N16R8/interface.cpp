#include "../../include/interface.h"
#include <Arduino.h>
#include <algorithm> // for std::max_element, std::min_element

// Externs for global variables, breaking dependency on globals.h
extern volatile bool PrevPress;
extern volatile bool NextPress;
extern volatile bool UpPress;
extern volatile bool DownPress;
extern volatile bool SelPress;
extern volatile bool AnyKeyPress;
extern volatile bool EscPress;

// Externs for functions defined elsewhere
extern void updateTouchPoint();
extern bool wakeUpScreen();

// ===== Runtime device detection flags =====
// These are set once during _setup_gpio() and never change after.
static bool joystickDetected = false;
static bool joystickButtonDetected = false;

/**
 * Detect if an analog joystick axis is physically connected.
 * A connected joystick at rest reads ~2048 (mid-range 12-bit ADC)
 * with very low variance between successive reads.
 * A floating (disconnected) pin produces erratic, high-variance readings.
 *
 * Returns true if a real joystick is detected on the given pin.
 */
static bool detectAnalogDevice(int pin, const char *name) {
  if (pin < 0)
    return false;

  // Set pin to INPUT_PULLUP. If disconnected, it will read 4095.
  // If connected to a joystick (~10k pot), it will read near center.
  pinMode(pin, INPUT_PULLUP);
  delay(10); // let it settle

  const int NUM_SAMPLES = 8;
  int samples[NUM_SAMPLES];
  int minVal = 4095, maxVal = 0;
  long sum = 0;

  for (int i = 0; i < NUM_SAMPLES; i++) {
    samples[i] = analogRead(pin);
    if (samples[i] < minVal)
      minVal = samples[i];
    if (samples[i] > maxVal)
      maxVal = samples[i];
    sum += samples[i];
    delayMicroseconds(500);
  }

  int avg = sum / NUM_SAMPLES;
  int range = maxVal - minVal;

  // With INPUT_PULLUP:
  // - Disconnected: avg ~ 4095, range very small.
  // - Connected (at rest): avg ~ 2048, range very small.
  // - Connected (moving): range larger.
  // - Floating (if pullup fails): range large.

  // Improved detection: check if avg is in joystick range (500-3500) and range
  // is small
  bool detected = (avg >= 500 && avg <= 3500 && range < 50);

  Serial.printf("[GPIO] %s (pin %d): avg=%d, range=%d -> %s\n", name, pin, avg,
                range,
                detected ? "DETECTED (joystick connected)"
                         : "NOT CONNECTED (check wiring or power)");

  // Additional debug info
  if (!detected) {
    if (avg > 3900) {
      Serial.printf("[GPIO]   -> Pin reading high (%d), likely disconnected or "
                    "5V power issue\n",
                    avg);
    } else if (range >= 50) {
      Serial.printf("[GPIO]   -> High variance (%d), pin may be floating\n",
                    range);
    } else {
      Serial.printf("[GPIO]   -> Unexpected reading, check connections\n");
    }
  }

  return detected;
}

/**
 * Detect if a digital button is physically connected (pulled-up).
 * A properly connected button with INPUT_PULLUP reads HIGH when not pressed.
 * A floating pin may read random values.
 *
 * We take multiple reads: if all are HIGH or all LOW consistently, it's likely
 * connected.
 */
static bool detectDigitalButton(int pin, const char *name) {
  if (pin < 0)
    return false;

  pinMode(pin, INPUT_PULLUP);
  delay(2); // let the pullup settle

  int highCount = 0;
  const int NUM_SAMPLES = 10;
  for (int i = 0; i < NUM_SAMPLES; i++) {
    if (digitalRead(pin) == HIGH)
      highCount++;
    delayMicroseconds(200);
  }

  // With INPUT_PULLUP and no press, should read HIGH consistently
  // A floating pin will read mixed HIGH/LOW
  bool detected = (highCount >= 9); // allow 1 glitch

  Serial.printf("[GPIO] %s (pin %d): %d/%d HIGH -> %s\n", name, pin, highCount,
                NUM_SAMPLES,
                detected ? "DETECTED" : "NOT CONNECTED (disabled)");

  return detected;
}

/***************************************************************************************
** Function name: _setup_gpio()
** Location: main.cpp
** Description:   initial setup for the device with auto-detection of
* peripherals
***************************************************************************************/
void _setup_gpio() {
  Serial.println("[GPIO] === Peripheral Auto-Detection ===");

  // ---- RGB LED ----
#ifdef RGB_LED
  if (RGB_LED >= 0) {
    pinMode(RGB_LED, OUTPUT);
    digitalWrite(RGB_LED, LOW);
    Serial.printf("[GPIO] RGB LED on pin %d configured\n", RGB_LED);
  }
#endif

  // ---- Status LED ----
#ifdef LED
  if (LED >= 0) {
    pinMode(LED, OUTPUT);
    digitalWrite(LED, LOW);
    Serial.printf("[GPIO] LED on pin %d configured\n", LED);
  }
#endif

  // ---- Joystick Analog Axes (auto-detect) ----
  bool joyX = false, joyY = false;
#ifdef JOY_X_PIN
  joyX = detectAnalogDevice(JOY_X_PIN, "Joystick X-axis");
#endif
#ifdef JOY_Y_PIN
  joyY = detectAnalogDevice(JOY_Y_PIN, "Joystick Y-axis");
#endif
  // Only enable joystick if BOTH axes are detected
  joystickDetected = joyX && joyY;
  if (!joystickDetected && (joyX || joyY)) {
    Serial.println(
        "[GPIO] WARNING: Only one joystick axis detected, disabling joystick");
  }

  // ---- Joystick Button (auto-detect) ----
#ifdef JOY_BTN_PIN
  if (JOY_BTN_PIN >= 0) {
    joystickButtonDetected =
        detectDigitalButton(JOY_BTN_PIN, "Joystick Button");
  }
#endif

  // ---- CC1101 GDO Pins (input only, safe even if disconnected) ----
#ifdef CC1101_GDO0
  if (CC1101_GDO0 >= 0) {
    pinMode(CC1101_GDO0, INPUT);
  }
#endif
#ifdef CC1101_GDO2
  if (CC1101_GDO2 >= 0) {
    pinMode(CC1101_GDO2, INPUT);
  }
#endif

  // ---- NRF24 CE Pin ----
#ifdef NRF24_CE_PIN
  if (NRF24_CE_PIN >= 0) {
    pinMode(NRF24_CE_PIN, OUTPUT);
  }
#endif

  // ---- SPI Chip Selects (CRITICAL: Pull HIGH to prevent SPI collisions) ----
  Serial.println("[GPIO] Securing SPI Bus (Pulling CS pins HIGH)...");
#ifdef TFT_CS
  if (TFT_CS >= 0) {
    pinMode(TFT_CS, OUTPUT);
    digitalWrite(TFT_CS, HIGH);
  }
#endif
#ifdef TOUCH_CS
  if (TOUCH_CS >= 0) {
    pinMode(TOUCH_CS, OUTPUT);
    digitalWrite(TOUCH_CS, HIGH);
  }
#endif
#ifdef SDCARD_CS
  if (SDCARD_CS >= 0) {
    pinMode(SDCARD_CS, OUTPUT);
    digitalWrite(SDCARD_CS, HIGH);
  }
#endif
#ifdef NRF24_CS_PIN
  if (NRF24_CS_PIN >= 0) {
    pinMode(NRF24_CS_PIN, OUTPUT);
    digitalWrite(NRF24_CS_PIN, HIGH);
  }
#endif
#ifdef NRF24_CS2_PIN
  if (NRF24_CS2_PIN >= 0) {
    pinMode(NRF24_CS2_PIN, OUTPUT);
    digitalWrite(NRF24_CS2_PIN, HIGH);
  }
#endif
#ifdef CC1101_CS_PIN
  if (CC1101_CS_PIN >= 0) {
    pinMode(CC1101_CS_PIN, OUTPUT);
    digitalWrite(CC1101_CS_PIN, HIGH);
  }
#endif

  Serial.printf("[GPIO] Summary: Joystick=%s, JoyBtn=%s\n",
                joystickDetected ? "YES" : "NO",
                joystickButtonDetected ? "YES" : "NO");
  Serial.println("[GPIO] === Auto-Detection Complete ===");
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
** Handles input from all detected devices (touch, joystick, buttons).
** Devices that were not detected at startup are safely skipped.
**********************************************************************/
void InputHandler(void) {
  static uint32_t last_check = 0;
  if (millis() - last_check < 20)
    return;
  last_check = millis();

  static bool btn_last_state = HIGH;

  // ---- Touch Input (always active if HAS_TOUCH) ----
#ifdef HAS_TOUCH
  updateTouchPoint();
  if (wakeUpScreen()) {
    AnyKeyPress = true;
  }
#endif

  // ---- Joystick Button (only if detected at startup) ----
#ifdef JOY_BTN_PIN
  if (joystickButtonDetected) {
    bool btn_state = digitalRead(JOY_BTN_PIN);
    if (btn_state == LOW && btn_last_state == HIGH) {
      SelPress = true;
      AnyKeyPress = true;
    }
    btn_last_state = btn_state;
  }
#endif

  // ---- Joystick Analog Axes (only if detected at startup) ----
  if (!joystickDetected) {
    // Log once per second if joystick not detected
    static uint32_t last_log = 0;
    if (millis() - last_log > 1000) {
      Serial.println("[JOY] Joystick not detected, skipping analog reads");
      last_log = millis();
    }
    return; // Skip all analog reads if no joystick
  }

#ifdef JOY_X_PIN
  if (JOY_X_PIN >= 0) {
    // Improved debounce: take 3 readings and use median
    int readings[3];
    for (int i = 0; i < 3; i++) {
      readings[i] = analogRead(JOY_X_PIN);
      delayMicroseconds(50);
    }
    // Simple median filter
    int x = readings[1]; // Middle reading as approximation
    // Check stability: max difference between readings
    int max_diff =
        max(abs(readings[0] - readings[1]), abs(readings[1] - readings[2]));
    if (max_diff < 100) { // More lenient stability check
      // Calibrated thresholds for KY-023
      if (x < 1200) { // Left (was 1000, increased for stability)
        PrevPress = true;
        AnyKeyPress = true;
        Serial.printf("[JOY] Left detected: X=%d\n", x);
      } else if (x > 3600) { // Right (was 3800, decreased for stability)
        NextPress = true;
        AnyKeyPress = true;
        Serial.printf("[JOY] Right detected: X=%d\n", x);
      }
    }
  }
#endif

#ifdef JOY_Y_PIN
  if (JOY_Y_PIN >= 0) {
    // Improved debounce: take 3 readings and use median
    int readings[3];
    for (int i = 0; i < 3; i++) {
      readings[i] = analogRead(JOY_Y_PIN);
      delayMicroseconds(50);
    }
    // Simple median filter
    int y = readings[1]; // Middle reading as approximation
    // Check stability: max difference between readings
    int max_diff =
        max(abs(readings[0] - readings[1]), abs(readings[1] - readings[2]));
    if (max_diff < 100) { // More lenient stability check
      // Calibrated thresholds for KY-023
      if (y < 1200) { // Up (was 1000, increased for stability)
        UpPress = true;
        AnyKeyPress = true;
        Serial.printf("[JOY] Up detected: Y=%d\n", y);
      } else if (y > 3600) { // Down (was 3800, decreased for stability)
        DownPress = true;
        AnyKeyPress = true;
        Serial.printf("[JOY] Down detected: Y=%d\n", y);
      }
    }
  }
#endif
}

// ===== Getter functions for joystick detection =====
bool getJoystickDetected() { return joystickDetected; }

bool getJoystickButtonDetected() { return joystickButtonDetected; }
