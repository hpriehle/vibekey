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
 * Features:
 *   - 3 remappable buttons via serial (no reflash needed)
 *   - Multiple profiles (switch with serial or button combo)
 *   - Macros: one button can send a sequence of keys
 *   - Persistent storage in ESP32 flash (NVS)
 *   - Battery level monitoring via ADC + voltage divider
 *   - Deep sleep on inactivity with button-press wakeup
 *
 * Serial protocol (115200 baud):
 *   PING                — returns "PONG:vibekey"
 *   GET                 — returns current mappings as "MAP:k1,k2,k3"
 *   SET k1,k2,k3        — sets key codes and saves to flash
 *   KEYS                — lists all available key names and codes
 *   PROFILE             — returns current profile number "PROFILE:n"
 *   PROFILE n           — switch to profile n (0-based)
 *   PROFILES            — returns number of profiles "PROFILES:n"
 *   MACRO b steps       — set macro for button b: "MACRO 0 128+97,0,97"
 *                         Each step is [modifier+]keycode. Steps separated by commas.
 *                         Use 0 as a step to insert a pause.
 *   GETMACRO b          — returns macro for button b
 *   CLEARMACRO b        — clear macro for button b (reverts to single key)
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

#if ENABLE_DEEP_SLEEP
#include <esp_sleep.h>
#endif

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

// -- PROFILE STATE --------------------------------------------
uint8_t currentProfile = 0;

// -- MACRO STATE ----------------------------------------------
// Each macro step: high byte = modifier, low byte = keycode
// A step of 0 means "pause" (MACRO_STEP_DELAY_MS)
// macroLen[i] == 0 means button i uses simple single-key mode
uint16_t macroSteps[NUM_BUTTONS][MAX_MACRO_STEPS];
uint8_t  macroLen[NUM_BUTTONS];

// -- LED STATE ------------------------------------------------
bool ledOn             = false;
unsigned long ledOffTime   = 0;
unsigned long lastBlinkToggle = 0;
bool blinkState        = false;

// -- ACTIVITY / SLEEP STATE -----------------------------------
unsigned long lastActivityTime = 0;

// -- BATTERY STATE --------------------------------------------
#if ENABLE_BATTERY_MONITOR
unsigned long lastBatteryRead = 0;
#endif

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

// -- BATTERY READING -----------------------------------------
#if ENABLE_BATTERY_MONITOR
int readBatteryPercent() {
  int raw = analogRead(BATTERY_PIN);
  float voltage = (raw / 4095.0) * 3.3 * VDIV_RATIO;
  int pct = (int)((voltage - BATT_MIN_V) / (BATT_MAX_V - BATT_MIN_V) * 100);
  return constrain(pct, 0, 100);
}
#endif

// -- PROFILE PERSISTENCE -------------------------------------

String profileNamespace() {
  return "vk_p" + String(currentProfile);
}

void loadMappings() {
  prefs.begin(profileNamespace().c_str(), true);  // read-only
  keyMappings[0] = prefs.getUChar("key0", DEFAULT_KEY_1);
  keyMappings[1] = prefs.getUChar("key1", DEFAULT_KEY_2);
  keyMappings[2] = prefs.getUChar("key2", DEFAULT_KEY_3);
  prefs.end();
}

void saveMappings() {
  prefs.begin(profileNamespace().c_str(), false);  // read-write
  prefs.putUChar("key0", keyMappings[0]);
  prefs.putUChar("key1", keyMappings[1]);
  prefs.putUChar("key2", keyMappings[2]);
  prefs.end();
}

void loadProfile(uint8_t p) {
  currentProfile = p;

  // Save active profile number
  prefs.begin("vibekey", false);
  prefs.putUChar("profile", currentProfile);
  prefs.end();

  loadMappings();
  loadMacros();
}

void loadMacros() {
  prefs.begin(profileNamespace().c_str(), true);
  for (int i = 0; i < NUM_BUTTONS; i++) {
    String lenKey = "ml" + String(i);
    macroLen[i] = prefs.getUChar(lenKey.c_str(), 0);
    if (macroLen[i] > MAX_MACRO_STEPS) macroLen[i] = 0;
    for (int s = 0; s < macroLen[i]; s++) {
      String stepKey = "ms" + String(i) + "_" + String(s);
      macroSteps[i][s] = prefs.getUShort(stepKey.c_str(), 0);
    }
  }
  prefs.end();
}

void saveMacro(int btn) {
  prefs.begin(profileNamespace().c_str(), false);
  String lenKey = "ml" + String(btn);
  prefs.putUChar(lenKey.c_str(), macroLen[btn]);
  for (int s = 0; s < macroLen[btn]; s++) {
    String stepKey = "ms" + String(btn) + "_" + String(s);
    prefs.putUShort(stepKey.c_str(), macroSteps[btn][s]);
  }
  prefs.end();
}

void clearMacro(int btn) {
  macroLen[btn] = 0;
  prefs.begin(profileNamespace().c_str(), false);
  String lenKey = "ml" + String(btn);
  prefs.putUChar(lenKey.c_str(), 0);
  prefs.end();
}

// -- KEY EXECUTION -------------------------------------------

void executeButton(int btn) {
  if (macroLen[btn] > 0) {
    // Execute macro sequence
    Serial.print("  -> macro (");
    Serial.print(macroLen[btn]);
    Serial.println(" steps)");
    for (int s = 0; s < macroLen[btn]; s++) {
      uint16_t step = macroSteps[btn][s];
      if (step == 0) {
        delay(MACRO_STEP_DELAY_MS);
        continue;
      }
      uint8_t modifier = (step >> 8) & 0xFF;
      uint8_t keycode = step & 0xFF;
      if (modifier) {
        bleKeyboard.press(modifier);
      }
      bleKeyboard.press(keycode);
      delay(MACRO_STEP_DELAY_MS);
      bleKeyboard.releaseAll();
    }
  } else {
    // Single key
    Serial.print("  -> sending key code ");
    Serial.println(keyMappings[btn]);
    bleKeyboard.write(keyMappings[btn]);
  }
  ledFlash();
}

// -- SERIAL PROTOCOL ------------------------------------------

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
  Serial.println("132,KEY_RIGHT_CTRL");
  Serial.println("133,KEY_RIGHT_SHIFT");
  Serial.println("134,KEY_RIGHT_ALT");
  Serial.println("135,KEY_RIGHT_GUI");
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
  Serial.println("212,KEY_HOME");
  Serial.println("213,KEY_PAGE_UP");
  Serial.println("214,KEY_DELETE");
  Serial.println("215,KEY_END");
  Serial.println("216,KEY_PAGE_DOWN");
  Serial.println("217,KEY_RIGHT_ARROW");
  Serial.println("218,KEY_LEFT_ARROW");
  Serial.println("219,KEY_DOWN_ARROW");
  Serial.println("220,KEY_UP_ARROW");
  Serial.println("226,KEY_MEDIA_MUTE");
  Serial.println("233,KEY_MEDIA_VOLUME_UP");
  Serial.println("234,KEY_MEDIA_VOLUME_DOWN");
  Serial.println("205,KEY_MEDIA_PLAY_PAUSE");
  Serial.println("181,KEY_MEDIA_NEXT_TRACK");
  Serial.println("182,KEY_MEDIA_PREVIOUS_TRACK");
  Serial.println("183,KEY_MEDIA_STOP");
  Serial.println("KEYS_END");
}

void printMacro(int btn) {
  Serial.print("MACRO:");
  Serial.print(btn);
  Serial.print(":");
  Serial.print(macroLen[btn]);
  Serial.print(":");
  for (int s = 0; s < macroLen[btn]; s++) {
    if (s > 0) Serial.print(",");
    uint16_t step = macroSteps[btn][s];
    uint8_t modifier = (step >> 8) & 0xFF;
    uint8_t keycode = step & 0xFF;
    if (modifier) {
      Serial.print(modifier);
      Serial.print("+");
    }
    Serial.print(keycode);
  }
  Serial.println();
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

  } else if (cmd == "PROFILE") {
    Serial.print("PROFILE:");
    Serial.println(currentProfile);

  } else if (cmd.startsWith("PROFILE ")) {
    int p = cmd.substring(8).toInt();
    if (p >= 0 && p < NUM_PROFILES) {
      loadProfile((uint8_t)p);
      Serial.print("OK:PROFILE:");
      Serial.println(currentProfile);
      printMappings();
    } else {
      Serial.print("ERR:profile must be 0-");
      Serial.println(NUM_PROFILES - 1);
    }

  } else if (cmd == "PROFILES") {
    Serial.print("PROFILES:");
    Serial.println(NUM_PROFILES);

  } else if (cmd.startsWith("MACRO ")) {
    // Format: MACRO <btn> <mod+key,mod+key,...>
    String args = cmd.substring(6);
    int space = args.indexOf(' ');
    if (space == -1) {
      Serial.println("ERR:usage MACRO <btn> <steps>");
      return;
    }
    int btn = args.substring(0, space).toInt();
    if (btn < 0 || btn >= NUM_BUTTONS) {
      Serial.println("ERR:invalid button");
      return;
    }
    String stepsStr = args.substring(space + 1);
    int stepIdx = 0;
    while (stepsStr.length() > 0 && stepIdx < MAX_MACRO_STEPS) {
      int comma = stepsStr.indexOf(',');
      String token;
      if (comma == -1) {
        token = stepsStr;
        stepsStr = "";
      } else {
        token = stepsStr.substring(0, comma);
        stepsStr = stepsStr.substring(comma + 1);
      }
      int plus = token.indexOf('+');
      if (plus != -1) {
        uint8_t mod = (uint8_t)token.substring(0, plus).toInt();
        uint8_t key = (uint8_t)token.substring(plus + 1).toInt();
        macroSteps[btn][stepIdx] = ((uint16_t)mod << 8) | key;
      } else {
        macroSteps[btn][stepIdx] = (uint16_t)token.toInt();
      }
      stepIdx++;
    }
    macroLen[btn] = stepIdx;
    saveMacro(btn);
    Serial.println("OK");
    printMacro(btn);

  } else if (cmd.startsWith("GETMACRO ")) {
    int btn = cmd.substring(9).toInt();
    if (btn >= 0 && btn < NUM_BUTTONS) {
      printMacro(btn);
    } else {
      Serial.println("ERR:invalid button");
    }

  } else if (cmd.startsWith("CLEARMACRO ")) {
    int btn = cmd.substring(11).toInt();
    if (btn >= 0 && btn < NUM_BUTTONS) {
      clearMacro(btn);
      Serial.println("OK");
    } else {
      Serial.println("ERR:invalid button");
    }

  } else if (cmd.length() > 0) {
    Serial.println("ERR:unknown command");
  }
}

// -- SETUP ----------------------------------------------------
void setup() {
  Serial.begin(115200);
  Serial.println("=============================");
  Serial.println("  VibeKey BLE Keyboard v5.0");
  Serial.println("=============================");

  // Load active profile number
  prefs.begin("vibekey", true);
  currentProfile = prefs.getUChar("profile", 0);
  if (currentProfile >= NUM_PROFILES) currentProfile = 0;
  prefs.end();

  // Load key mappings and macros for active profile
  loadMappings();
  loadMacros();
  Serial.print("Profile: ");
  Serial.print(currentProfile);
  Serial.print("/");
  Serial.println(NUM_PROFILES - 1);
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
    Serial.print(keyMappings[i]);
    if (macroLen[i] > 0) {
      Serial.print(" (macro: ");
      Serial.print(macroLen[i]);
      Serial.print(" steps)");
    }
    Serial.println();
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

  // Configure deep sleep wakeup sources (all button pins, wake on LOW)
#if ENABLE_DEEP_SLEEP
  uint64_t wakeupMask = (1ULL << BUTTON_PIN_1) | (1ULL << BUTTON_PIN_2) | (1ULL << BUTTON_PIN_3);
  esp_sleep_enable_ext1_wakeup(wakeupMask, ESP_EXT1_WAKEUP_ALL_LOW);
  Serial.println("Deep sleep enabled (wakeup on any button press).");
#endif

  // Initialize activity timer
  lastActivityTime = millis();

#if ENABLE_BATTERY_MONITOR
  // Take an initial battery reading
  int battPct = readBatteryPercent();
  bleKeyboard.setBatteryLevel(battPct);
  lastBatteryRead = millis();
  Serial.print("Battery: ");
  Serial.print(battPct);
  Serial.println("%");
#endif

  // Check for profile-cycle combo: hold Button 1 + Button 3 during boot
  delay(200);
  if (digitalRead(buttonPins[0]) == LOW && digitalRead(buttonPins[NUM_BUTTONS - 1]) == LOW) {
    uint8_t nextProfile = (currentProfile + 1) % NUM_PROFILES;
    Serial.print("Profile combo detected! Switching to profile ");
    Serial.println(nextProfile);
    loadProfile(nextProfile);

    // Flash LED to indicate profile number (nextProfile + 1 blinks)
#if LED_PIN >= 0
    for (int i = 0; i <= (int)nextProfile; i++) {
      ledSet(true);
      delay(200);
      ledSet(false);
      delay(200);
    }
#endif
  }
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
        lastActivityTime = now;
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
        lastActivityTime = now;
        Serial.print("Button ");
        Serial.print(i + 1);
        Serial.println(" DOWN");

        if (connected) {
          executeButton(i);
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

  // ---- Battery level reporting ----
#if ENABLE_BATTERY_MONITOR
  if ((now - lastBatteryRead) >= BATTERY_READ_INTERVAL_MS) {
    lastBatteryRead = now;
    int battPct = readBatteryPercent();
    bleKeyboard.setBatteryLevel(battPct);
    Serial.print("Battery: ");
    Serial.print(battPct);
    Serial.println("%");
  }
#endif

  // ---- Deep sleep on inactivity ----
#if ENABLE_DEEP_SLEEP
  if ((now - lastActivityTime) >= SLEEP_TIMEOUT_MS) {
    Serial.println("Inactivity timeout — entering deep sleep...");
    Serial.flush();
    ledSet(false);
    esp_deep_sleep_start();
  }
#endif

  delay(1);
}
