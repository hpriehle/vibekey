/*
 * VibeKey BLE Keyboard
 * ====================
 * Turns an ESP32 into a 3-button Bluetooth keyboard.
 *
 * Hardware:
 *   - ESP32 dev board (ESP32-S3 recommended)
 *   - 3 MX-compatible mechanical key switches
 *   - Each switch wired between its GPIO pin and GND
 *   - (Optional) LED on LED_PIN for status feedback
 *
 * Key mappings are stored in ESP32 flash (Preferences/NVS) and can
 * be changed at runtime via serial commands — no reflashing needed.
 * Use the Python mapper tool: python3 mapper/mapper.py
 *
 * Serial protocol (115200 baud):
 *   GET              — returns current mappings as "MAP:k1,k2,k3"
 *   SET k1,k2,k3     — sets key codes (decimal) and saves to flash
 *   KEYS             — lists all available key names and codes
 *
 * Library required:
 *   ESP32-BLE-Keyboard by T-vK
 *   https://github.com/T-vK/ESP32-BLE-Keyboard
 *
 * All pin/LED/BLE settings are in config.h.
 */

#include <BleKeyboard.h>
#include <Preferences.h>
#include "config.h"

// -- BLE KEYBOARD INSTANCE ------------------------------------
BleKeyboard bleKeyboard(BLE_DEVICE_NAME, BLE_MANUFACTURER, BLE_BATTERY);

// -- PERSISTENT STORAGE ---------------------------------------
Preferences prefs;

// -- BUTTON STATE (per-button arrays) -------------------------
const int buttonPins[NUM_BUTTONS] = { BUTTON_PIN_1, BUTTON_PIN_2, BUTTON_PIN_3 };
uint8_t   keyMappings[NUM_BUTTONS];

bool          lastReading[NUM_BUTTONS];
bool          stableState[NUM_BUTTONS];
unsigned long debounceStart[NUM_BUTTONS];

// -- LED STATE ------------------------------------------------
bool ledOn             = false;
unsigned long ledOffTime   = 0;
unsigned long lastBlinkToggle = 0;
bool blinkState        = false;

// -- SERIAL INPUT BUFFER --------------------------------------
String serialBuffer = "";

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

void loadMappings() {
  prefs.begin("vibekey", true);  // read-only
  keyMappings[0] = prefs.getUChar("key0", DEFAULT_KEY_1);
  keyMappings[1] = prefs.getUChar("key1", DEFAULT_KEY_2);
  keyMappings[2] = prefs.getUChar("key2", DEFAULT_KEY_3);
  prefs.end();
}

void saveMappings() {
  prefs.begin("vibekey", false);  // read-write
  prefs.putUChar("key0", keyMappings[0]);
  prefs.putUChar("key1", keyMappings[1]);
  prefs.putUChar("key2", keyMappings[2]);
  prefs.end();
}

void printMappings() {
  Serial.print("MAP:");
  for (int i = 0; i < NUM_BUTTONS; i++) {
    if (i > 0) Serial.print(",");
    Serial.print(keyMappings[i]);
  }
  Serial.println();
}

void printKeyList() {
  Serial.println("KEYS_BEGIN");
  Serial.println("128,KEY_LEFT_CTRL");
  Serial.println("129,KEY_LEFT_SHIFT");
  Serial.println("130,KEY_LEFT_ALT");
  Serial.println("131,KEY_LEFT_GUI");
  Serial.println("176,KEY_RETURN");
  Serial.println("177,KEY_ESC");
  Serial.println("178,KEY_BACKSPACE");
  Serial.println("179,KEY_TAB");
  Serial.println("32,SPACE");
  Serial.println("193,KEY_CAPS_LOCK");
  Serial.println("194,KEY_F1");
  Serial.println("195,KEY_F2");
  Serial.println("196,KEY_F3");
  Serial.println("197,KEY_F4");
  Serial.println("198,KEY_F5");
  Serial.println("199,KEY_F6");
  Serial.println("200,KEY_F7");
  Serial.println("201,KEY_F8");
  Serial.println("202,KEY_F9");
  Serial.println("203,KEY_F10");
  Serial.println("204,KEY_F11");
  Serial.println("205,KEY_F12");
  Serial.println("206,KEY_F13");
  Serial.println("207,KEY_F14");
  Serial.println("208,KEY_F15");
  Serial.println("209,KEY_F16");
  Serial.println("210,KEY_F17");
  Serial.println("211,KEY_F18");
  Serial.println("212,KEY_F19");
  Serial.println("213,KEY_F20");
  Serial.println("214,KEY_F21");
  Serial.println("215,KEY_F22");
  Serial.println("216,KEY_F23");
  Serial.println("217,KEY_F24");
  Serial.println("218,KEY_INSERT");
  Serial.println("212,KEY_HOME");
  Serial.println("213,KEY_PAGE_UP");
  Serial.println("214,KEY_DELETE");
  Serial.println("215,KEY_END");
  Serial.println("216,KEY_PAGE_DOWN");
  Serial.println("217,KEY_RIGHT_ARROW");
  Serial.println("218,KEY_LEFT_ARROW");
  Serial.println("219,KEY_DOWN_ARROW");
  Serial.println("220,KEY_UP_ARROW");
  Serial.println("205,KEY_MEDIA_PLAY_PAUSE");
  Serial.println("226,KEY_MEDIA_MUTE");
  Serial.println("233,KEY_MEDIA_VOLUME_UP");
  Serial.println("234,KEY_MEDIA_VOLUME_DOWN");
  Serial.println("181,KEY_MEDIA_NEXT_TRACK");
  Serial.println("182,KEY_MEDIA_PREVIOUS_TRACK");
  Serial.println("183,KEY_MEDIA_STOP");
  Serial.println("KEYS_END");
}

void handleSerialCommand(String cmd) {
  cmd.trim();

  if (cmd == "GET") {
    printMappings();

  } else if (cmd.startsWith("SET ")) {
    String args = cmd.substring(4);
    int values[NUM_BUTTONS];
    int idx = 0;

    while (args.length() > 0 && idx < NUM_BUTTONS) {
      int comma = args.indexOf(',');
      if (comma == -1) {
        values[idx] = args.toInt();
        args = "";
      } else {
        values[idx] = args.substring(0, comma).toInt();
        args = args.substring(comma + 1);
      }
      idx++;
    }

    if (idx == NUM_BUTTONS) {
      for (int i = 0; i < NUM_BUTTONS; i++) {
        keyMappings[i] = (uint8_t)values[i];
      }
      saveMappings();
      Serial.println("OK");
      printMappings();
    } else {
      Serial.print("ERR:expected ");
      Serial.print(NUM_BUTTONS);
      Serial.println(" values");
    }

  } else if (cmd == "KEYS") {
    printKeyList();

  } else if (cmd == "PING") {
    Serial.println("PONG:vibekey");

  } else if (cmd.length() > 0) {
    Serial.println("ERR:unknown command");
  }
}

// -- SETUP ----------------------------------------------------
void setup() {
  Serial.begin(115200);
  Serial.println("=============================");
  Serial.println("  VibeKey BLE Keyboard v3.0");
  Serial.println("=============================");

  // Load key mappings from flash
  loadMappings();
  Serial.print("Loaded mappings: ");
  printMappings();

  // Initialize button pins
  for (int i = 0; i < NUM_BUTTONS; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
    lastReading[i]   = HIGH;
    stableState[i]   = HIGH;
    debounceStart[i] = 0;
    Serial.print("Button ");
    Serial.print(i + 1);
    Serial.print(" on GPIO ");
    Serial.print(buttonPins[i]);
    Serial.print(" -> key code ");
    Serial.println(keyMappings[i]);
  }

#if LED_PIN >= 0
  pinMode(LED_PIN, OUTPUT);
  ledSet(false);
  Serial.print("LED on GPIO ");
  Serial.println(LED_PIN);
#endif

  Serial.println("Initializing BLE...");
  bleKeyboard.begin();
  Serial.println("BLE advertising started.");
  Serial.println("Waiting for connection...");
}

// -- MAIN LOOP ------------------------------------------------
void loop() {
  unsigned long now = millis();
  bool connected = bleKeyboard.isConnected();

  // ---- Handle serial commands ----
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (serialBuffer.length() > 0) {
        handleSerialCommand(serialBuffer);
        serialBuffer = "";
      }
    } else {
      serialBuffer += c;
    }
  }

  // ---- Read and debounce each button ----
  for (int i = 0; i < NUM_BUTTONS; i++) {
    bool reading = digitalRead(buttonPins[i]);

    if (reading != lastReading[i]) {
      debounceStart[i] = now;
    }
    lastReading[i] = reading;

    if ((now - debounceStart[i]) > DEBOUNCE_MS && reading != stableState[i]) {
      stableState[i] = reading;

      if (stableState[i] == LOW) {
        // Button i just pressed
        Serial.print("Button ");
        Serial.print(i + 1);
        Serial.println(" DOWN");

        if (connected) {
          Serial.print("  -> sending key code ");
          Serial.println(keyMappings[i]);
          bleKeyboard.write(keyMappings[i]);
          ledFlash();
        } else {
          Serial.println("  -> BLE not connected");
        }
      } else {
        Serial.print("Button ");
        Serial.print(i + 1);
        Serial.println(" UP");
      }
    }
  }

  // ---- LED management ----
#if LED_PIN >= 0
  if (ledOffTime > 0 && now >= ledOffTime) {
    ledOffTime = 0;
#if LED_MODE == LED_MODE_KEYPRESS
    ledSet(false);
#elif LED_MODE == LED_MODE_CONNECTION
    ledSet(connected);
#elif LED_MODE == LED_MODE_BOTH
    if (connected) {
      ledSet(false);
    }
#endif
  }

#if LED_MODE == LED_MODE_CONNECTION
  if (ledOffTime == 0) {
    ledSet(connected);
  }
#elif LED_MODE == LED_MODE_BOTH
  if (!connected && ledOffTime == 0) {
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
