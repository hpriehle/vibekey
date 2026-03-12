/*
 * OneKey BLE Keyboard — User Configuration
 * =========================================
 * Edit this file to customize your key, pins, and behavior.
 * No need to touch one_key_keyboard.ino unless you want to
 * change the core logic.
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <BleKeyboard.h>

// -----------------------------------------------------------------
// PIN ASSIGNMENTS
// -----------------------------------------------------------------

// GPIO pin wired to the key switch (other switch pin goes to GND)
#define BUTTON_PIN      4

// GPIO pin for the status LED (set to -1 to disable)
// The built-in LED on most ESP32-S3 DevKits is GPIO 48.
// For original ESP32 DevKit, try GPIO 2.
#define LED_PIN         2

// -----------------------------------------------------------------
// BLUETOOTH
// -----------------------------------------------------------------

// Device name shown in Bluetooth pairing (max 15 characters)
#define BLE_DEVICE_NAME "OneKey"

// Manufacturer name in BLE device info
#define BLE_MANUFACTURER "DIY"

// Battery level reported to host (0-100, cosmetic only)
#define BLE_BATTERY     100

// -----------------------------------------------------------------
// KEY ACTION — SINGLE PRESS
// -----------------------------------------------------------------
// What to send when the key is pressed once.
//
//   KEY_MEDIA_MUTE          Mute/unmute (Zoom, Teams, etc.)
//   KEY_MEDIA_PLAY_PAUSE    Play/pause media
//   KEY_MEDIA_VOLUME_UP     Volume up
//   KEY_MEDIA_VOLUME_DOWN   Volume down
//   KEY_MEDIA_NEXT_TRACK    Next track
//   KEY_MEDIA_PREVIOUS_TRACK Previous track
//   KEY_F13                 F13 (great for push-to-talk binds)
//   KEY_F24                 F24 (another unused key for macros)
//   KEY_RETURN              Enter
//   KEY_ESC                 Escape
//   'a'                     Letter a (any single character)

#define KEY_TO_SEND     KEY_MEDIA_MUTE

// -----------------------------------------------------------------
// KEY ACTION — LONG PRESS (hold > threshold)
// -----------------------------------------------------------------
// Set to true to enable a second action on long press.
// When enabled, the single-press action fires on release (short tap),
// and the long-press action fires after the threshold.

#define LONG_PRESS_ENABLED   false
#define LONG_PRESS_MS        500
#define LONG_PRESS_KEY       KEY_MEDIA_PLAY_PAUSE

// -----------------------------------------------------------------
// KEY COMBO MODE
// -----------------------------------------------------------------
// Set to true to send a multi-key combo instead of a single key.
// When enabled, KEY_TO_SEND is ignored — edit sendCombo() in the
// .ino file to define your combo.
// Default combo: Ctrl+Shift+M (Zoom mute toggle)

#define USE_COMBO       false

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
