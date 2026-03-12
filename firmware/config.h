/*
 * VibeKey BLE Keyboard — User Configuration
 * ==========================================
 * Edit this file to customize pins, Bluetooth name, and behavior.
 * Key mappings are stored on the ESP32 and configured via the
 * CLI tool (vibekey map) — no need to reflash to change keys.
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <BleKeyboard.h>

// -----------------------------------------------------------------
// NUMBER OF BUTTONS
// -----------------------------------------------------------------
#define NUM_BUTTONS     3

// -----------------------------------------------------------------
// PIN ASSIGNMENTS
// -----------------------------------------------------------------
// GPIO pins wired to each key switch (other switch pin goes to GND).
// Change these to match your wiring.

#define BUTTON_PIN_1    4
#define BUTTON_PIN_2    5
#define BUTTON_PIN_3    6

// GPIO pin for the status LED (set to -1 to disable)
// ESP32-S3 DevKit built-in LED: GPIO 48. Original ESP32 DevKit: GPIO 2.
#define LED_PIN         2

// -----------------------------------------------------------------
// BLUETOOTH
// -----------------------------------------------------------------

// Device name shown in Bluetooth pairing (max 15 characters)
#define BLE_DEVICE_NAME "VibeKey"

// Manufacturer name in BLE device info
#define BLE_MANUFACTURER "DIY"

// Battery level reported to host (0-100, cosmetic only)
#define BLE_BATTERY     100

// -----------------------------------------------------------------
// DEFAULT KEY MAPPINGS
// -----------------------------------------------------------------
// These are used on first boot only. After that, mappings are stored
// in flash and configured via the CLI tool or serial commands.
//
// Key code reference:
//   0xB0  KEY_RETURN            Enter
//   0xB1  KEY_ESC               Escape
//   0xB3  KEY_TAB               Tab
//   0xCD  KEY_MEDIA_PLAY_PAUSE  Play/pause media
//   0xE2  KEY_MEDIA_MUTE        Mute/unmute
//   0xE9  KEY_MEDIA_VOLUME_UP   Volume up
//   0xEA  KEY_MEDIA_VOLUME_DOWN Volume down
//   0xF0  KEY_F1  ...  0xFB KEY_F12
//   'a'-'z'                     Letters

#define DEFAULT_KEY_1   KEY_MEDIA_MUTE
#define DEFAULT_KEY_2   KEY_MEDIA_VOLUME_DOWN
#define DEFAULT_KEY_3   KEY_MEDIA_VOLUME_UP

// -----------------------------------------------------------------
// PROFILES
// -----------------------------------------------------------------
// Number of mapping profiles. Each profile stores a separate set of
// key mappings. Switch profiles via serial or button combo on boot.
// Hold Button 1 + Button 3 during the first second of boot to cycle.

#define NUM_PROFILES    3

// -----------------------------------------------------------------
// MACROS
// -----------------------------------------------------------------
// Maximum number of key steps in a macro sequence.
// A macro sends multiple keys in order with configurable delays.

#define MAX_MACRO_STEPS 8

// Delay between macro key presses (ms)
#define MACRO_STEP_DELAY_MS 50

// -----------------------------------------------------------------
// DEBOUNCE
// -----------------------------------------------------------------
// How long (ms) the switch state must be stable before registering.
// Increase to 80–100 if you get double-triggers.

#define DEBOUNCE_MS     50

// -----------------------------------------------------------------
// LED BEHAVIOR
// -----------------------------------------------------------------
// LED_MODE_CONNECTION : LED on = BLE connected, off = disconnected
// LED_MODE_KEYPRESS   : LED flashes briefly on each keypress
// LED_MODE_BOTH       : Slow blink when disconnected, flash on press

#define LED_MODE_CONNECTION  0
#define LED_MODE_KEYPRESS    1
#define LED_MODE_BOTH        2

#define LED_MODE        LED_MODE_BOTH

// Duration (ms) of the LED flash on keypress
#define LED_FLASH_MS    100

// Slow-blink interval (ms) when BLE is disconnected
#define LED_BLINK_MS    1000

#endif // CONFIG_H
