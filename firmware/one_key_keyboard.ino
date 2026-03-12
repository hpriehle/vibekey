/*
 * OneKey BLE Keyboard
 * ===================
 * Turns an ESP32 into a single-key Bluetooth keyboard.
 *
 * Hardware:
 *   - ESP32 dev board (ESP32-S3 recommended)
 *   - MX-compatible mechanical key switch wired between BUTTON_PIN and GND
 *   - (Optional) LED on LED_PIN for status feedback
 *
 * Library required:
 *   ESP32-BLE-Keyboard by T-vK
 *   https://github.com/T-vK/ESP32-BLE-Keyboard
 *
 * All user-configurable settings are in config.h.
 */

#include <BleKeyboard.h>
#include "config.h"

// -- BLE KEYBOARD INSTANCE ------------------------------------
BleKeyboard bleKeyboard(BLE_DEVICE_NAME, BLE_MANUFACTURER, BLE_BATTERY);

// -- BUTTON STATE ---------------------------------------------
bool lastReading       = HIGH;   // Previous raw reading
bool stableState       = HIGH;   // Debounced stable state
unsigned long debounceStart = 0;

// -- LONG PRESS STATE -----------------------------------------
bool keyIsDown         = false;  // True while key is physically held
bool longPressFired    = false;  // True if long-press already sent
unsigned long pressStart = 0;    // When the key was pressed

// -- LED STATE ------------------------------------------------
bool ledOn             = false;
unsigned long ledOffTime   = 0;  // When to turn LED off after flash
unsigned long lastBlinkToggle = 0;
bool blinkState        = false;

// -- HELPERS --------------------------------------------------

void ledSet(bool on) {
#if LED_PIN >= 0
  digitalWrite(LED_PIN, on ? HIGH : LOW);
  ledOn = on;
#endif
}

void ledFlash() {
#if LED_PIN >= 0
  ledSet(true);
  ledOffTime = millis() + LED_FLASH_MS;
#endif
}

// Key combo — edit this for your custom multi-key shortcut.
// Default: Ctrl+Shift+M (Zoom/Teams mute toggle)
void sendCombo() {
  bleKeyboard.press(KEY_LEFT_CTRL);
  bleKeyboard.press(KEY_LEFT_SHIFT);
  bleKeyboard.press('m');
  delay(50);
  bleKeyboard.releaseAll();
}

void sendKey(uint8_t key) {
  if (USE_COMBO) {
    sendCombo();
  } else {
    bleKeyboard.write(key);
  }
}

// -- SETUP ----------------------------------------------------
void setup() {
  Serial.begin(115200);
  Serial.println("=============================");
  Serial.println("  OneKey BLE Keyboard v2.0");
  Serial.println("=============================");

  pinMode(BUTTON_PIN, INPUT_PULLUP);

#if LED_PIN >= 0
  pinMode(LED_PIN, OUTPUT);
  ledSet(false);
  Serial.print("LED on GPIO ");
  Serial.println(LED_PIN);
#endif

  Serial.print("Button on GPIO ");
  Serial.println(BUTTON_PIN);
  Serial.println("Initializing BLE...");

  bleKeyboard.begin();

  Serial.println("BLE advertising started.");
  Serial.println("Waiting for connection...");
}

// -- MAIN LOOP ------------------------------------------------
void loop() {
  unsigned long now = millis();
  bool connected = bleKeyboard.isConnected();

  // ---- Read and debounce the switch ----
  bool reading = digitalRead(BUTTON_PIN);

  if (reading != lastReading) {
    debounceStart = now;
  }
  lastReading = reading;

  if ((now - debounceStart) > DEBOUNCE_MS && reading != stableState) {
    stableState = reading;

    if (stableState == LOW) {
      // Key just pressed
      keyIsDown = true;
      longPressFired = false;
      pressStart = now;
      Serial.println("Key DOWN");

#if !LONG_PRESS_ENABLED
      // Immediate fire when long-press is disabled
      if (connected) {
        Serial.println("  -> sending keystroke");
        sendKey(KEY_TO_SEND);
        ledFlash();
      } else {
        Serial.println("  -> BLE not connected");
      }
#endif

    } else {
      // Key just released
      Serial.println("Key UP");

#if LONG_PRESS_ENABLED
      if (!longPressFired && connected) {
        // Short press — fire on release
        unsigned long held = now - pressStart;
        Serial.print("  -> short press (");
        Serial.print(held);
        Serial.println("ms) -> sending single key");
        sendKey(KEY_TO_SEND);
        ledFlash();
      }
#endif

      keyIsDown = false;
      longPressFired = false;
    }
  }

  // ---- Long-press detection (while held) ----
#if LONG_PRESS_ENABLED
  if (keyIsDown && !longPressFired && (now - pressStart) >= LONG_PRESS_MS) {
    longPressFired = true;
    if (connected) {
      Serial.println("  -> LONG press -> sending long-press key");
      bleKeyboard.write(LONG_PRESS_KEY);
      ledFlash();
    }
  }
#endif

  // ---- LED management ----
#if LED_PIN >= 0
  // Turn off flash LED after duration
  if (ledOffTime > 0 && now >= ledOffTime) {
    ledOffTime = 0;
    // Only turn off if we're not in connection-solid mode
#if LED_MODE == LED_MODE_KEYPRESS
    ledSet(false);
#elif LED_MODE == LED_MODE_CONNECTION
    ledSet(connected);
#elif LED_MODE == LED_MODE_BOTH
    if (connected) {
      ledSet(false);
    }
    // If disconnected, the blink loop below will take over
#endif
  }

#if LED_MODE == LED_MODE_CONNECTION
  if (ledOffTime == 0) {
    ledSet(connected);
  }
#elif LED_MODE == LED_MODE_BOTH
  if (!connected && ledOffTime == 0) {
    // Slow blink while disconnected
    if ((now - lastBlinkToggle) >= LED_BLINK_MS) {
      lastBlinkToggle = now;
      blinkState = !blinkState;
      ledSet(blinkState);
    }
  } else if (connected && ledOffTime == 0) {
    ledSet(false);
  }
#endif

#endif // LED_PIN >= 0

  delay(1);
}
