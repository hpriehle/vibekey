/*
 * OneKey BLE Keyboard
 * ===================
 * Turns an ESP32 into a single-key Bluetooth keyboard.
 *
 * Hardware:
 *   - ESP32 dev board (ESP32-S3 recommended)
 *   - MX-compatible mechanical key switch
 *   - Wire switch between GPIO 4 and GND
 *
 * Library required:
 *   ESP32-BLE-Keyboard by T-vK
 *   https://github.com/T-vK/ESP32-BLE-Keyboard
 *
 * Instructions:
 *   1. Install the ESP32-BLE-Keyboard library (Add .ZIP Library)
 *   2. Select your ESP32 board in Tools -> Board
 *   3. Upload this sketch
 *   4. Pair "OneKey" from your device's Bluetooth settings
 *   5. Press the key!
 */

#include <BleKeyboard.h>

// -- CONFIGURATION -------------------------------------------
const int BUTTON_PIN = 4;           // GPIO pin for the key switch
const int DEBOUNCE_MS = 50;         // Debounce delay in milliseconds

// What to send when the key is pressed.
// Change this to whatever you need!
//
// Common options:
//   KEY_RETURN              -> Enter key
//   KEY_MEDIA_MUTE          -> Mute/unmute microphone
//   KEY_MEDIA_PLAY_PAUSE    -> Play/pause media
//   KEY_MEDIA_VOLUME_UP     -> Volume up
//   KEY_MEDIA_VOLUME_DOWN   -> Volume down
//   KEY_MEDIA_NEXT_TRACK    -> Next track
//   KEY_MEDIA_PREVIOUS_TRACK-> Previous track
//   KEY_F13                 -> F13 (great for push-to-talk binds)
//   KEY_F24                 -> F24 (another unused key for macros)
//   'a'                     -> The letter 'a'
//
// For key combos (e.g., Ctrl+Shift+M), see the sendCombo()
// function below.

const uint8_t KEY_TO_SEND = KEY_MEDIA_MUTE;

// Set to true to send a key combo instead of a single key
const bool USE_COMBO = false;

// Bluetooth device name (max 15 characters)
BleKeyboard bleKeyboard("OneKey", "DIY", 100);

// -- STATE ---------------------------------------------------
bool lastButtonState = HIGH;        // HIGH = not pressed (pull-up)
bool currentButtonState = HIGH;
unsigned long lastDebounceTime = 0;

// -- KEY COMBO FUNCTION --------------------------------------
// Modify this if you want to send a multi-key combination.
// This example sends Ctrl+Shift+M (Zoom mute toggle).
void sendCombo() {
  bleKeyboard.press(KEY_LEFT_CTRL);
  bleKeyboard.press(KEY_LEFT_SHIFT);
  bleKeyboard.press('m');
  delay(50);
  bleKeyboard.releaseAll();
}

// -- SETUP ---------------------------------------------------
void setup() {
  Serial.begin(115200);
  Serial.println("=============================");
  Serial.println("  OneKey BLE Keyboard v1.0");
  Serial.println("=============================");
  Serial.println("Initializing BLE...");

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  bleKeyboard.begin();

  Serial.println("BLE advertising started.");
  Serial.println("Waiting for connection...");
}

// -- MAIN LOOP -----------------------------------------------
void loop() {
  bool reading = digitalRead(BUTTON_PIN);

  // Reset debounce timer on any state change
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  // Only act after the debounce period has elapsed
  if ((millis() - lastDebounceTime) > DEBOUNCE_MS) {
    if (reading != currentButtonState) {
      currentButtonState = reading;

      // Key was just pressed (LOW because of pull-up to GND wiring)
      if (currentButtonState == LOW) {
        if (bleKeyboard.isConnected()) {
          Serial.println("Key pressed -> sending keystroke");

          if (USE_COMBO) {
            sendCombo();
          } else {
            bleKeyboard.write(KEY_TO_SEND);
          }
        } else {
          Serial.println("Key pressed, but BLE not connected.");
        }
      }
    }
  }

  lastButtonState = reading;
  delay(1);  // Small delay to reduce CPU usage
}
