# OneKey — Single-Key Bluetooth Keyboard

**ESP32-Powered · 3D Printed · BLE HID**

Difficulty: Beginner | Build Time: 2–4 hours | Cost: ~$15

---

## What Is This?

A single mechanical key that connects to your computer, phone, or tablet over Bluetooth. Press it and it sends whatever keystroke you configure — mute your mic, play/pause media, trigger push-to-talk, or fire any key combo you want. Your device sees it as a standard Bluetooth keyboard. No drivers, no special software.

| Detail          | Spec                                                         |
|-----------------|--------------------------------------------------------------|
| Microcontroller | ESP32-S3 DevKitC-1 (recommended) or any ESP32 dev board     |
| Connectivity    | Bluetooth Low Energy (BLE) HID — no drivers needed           |
| Power           | USB-C (bus-powered from your computer or a power bank)       |
| Key Switch      | Any MX-compatible mechanical switch (Cherry, Gateron, Kailh) |
| Firmware        | Arduino / PlatformIO + ESP32-BLE-Keyboard library            |
| Case            | 3D-printed enclosure (OpenSCAD parametric source included)   |

---

## Project Structure

```
vibekey/
├── README.md                       # This file
├── HARDWARE.md                     # Step-by-step hardware assembly guide
├── firmware/
│   ├── one_key_keyboard.ino        # Main firmware (Arduino sketch)
│   └── config.h                    # All user-configurable settings
├── case/
│   └── one_key_case.scad           # OpenSCAD parametric 3D model
├── platformio.ini                  # PlatformIO build configuration
├── flash.sh                        # One-command build & upload script
└── .gitignore
```

---

## Quick Start

### 1. Get the Parts

- ESP32-S3 DevKitC-1 (~$8–10)
- MX-compatible key switch + keycap (~$2–3)
- USB-C cable
- Two short wires (22–26 AWG)
- (Optional) LED + 220Ω resistor for status indicator

### 2. Wire It Up

Two wires. That's it.

| Wire   | From             | To         |
|--------|------------------|------------|
| Wire 1 | Switch pin A     | ESP32 GPIO 4 |
| Wire 2 | Switch pin B     | ESP32 GND    |

For the full step-by-step with photos-style ASCII diagrams, soldering instructions, LED wiring, case assembly, and troubleshooting, see **[HARDWARE.md](HARDWARE.md)**.

### 3. Flash the Firmware

**Option A — PlatformIO (recommended):**

```bash
pip install platformio    # one-time setup
./flash.sh                # build + upload
./flash.sh monitor        # build + upload + serial monitor
```

**Option B — Arduino IDE:**

1. Install [Arduino IDE 2.x](https://www.arduino.cc/en/software)
2. Add ESP32 board support — in **File → Preferences**, add this URL:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. Install "esp32" in **Tools → Board → Boards Manager**
4. Install the BLE Keyboard library — download the `.zip` from [T-vK/ESP32-BLE-Keyboard releases](https://github.com/T-vK/ESP32-BLE-Keyboard/releases), then **Sketch → Include Library → Add .ZIP Library**
5. Open `firmware/one_key_keyboard.ino`
6. Select your board (**Tools → Board → ESP32S3 Dev Module**) and port
7. Set **USB CDC On Boot** to "Enabled" (ESP32-S3 only)
8. Click Upload

### 4. Pair and Press

1. Plug in the ESP32 via USB
2. Go to Bluetooth settings on your device
3. Pair **"OneKey"** — no PIN required
4. Press the key!

---

## Configuration

All settings live in [`firmware/config.h`](firmware/config.h). Edit and re-flash to change behavior.

### Change the Key

```cpp
#define KEY_TO_SEND     KEY_MEDIA_MUTE
```

| Constant                 | Action                                    |
|--------------------------|-------------------------------------------|
| `KEY_MEDIA_MUTE`         | Mute/unmute mic (Zoom, Teams, etc.)       |
| `KEY_MEDIA_PLAY_PAUSE`   | Play/pause media                          |
| `KEY_MEDIA_VOLUME_UP`    | Volume up                                 |
| `KEY_MEDIA_VOLUME_DOWN`  | Volume down                               |
| `KEY_MEDIA_NEXT_TRACK`   | Next track                                |
| `KEY_F13`                | F13 — ideal for push-to-talk binds        |
| `KEY_RETURN`             | Enter                                     |
| `'a'`                    | Any single character                      |

### Key Combos

For multi-key shortcuts (e.g., Ctrl+Shift+M for Zoom mute):

```cpp
#define USE_COMBO  true
```

Then edit the `sendCombo()` function in `one_key_keyboard.ino`:

```cpp
void sendCombo() {
  bleKeyboard.press(KEY_LEFT_CTRL);
  bleKeyboard.press(KEY_LEFT_SHIFT);
  bleKeyboard.press('m');
  delay(50);
  bleKeyboard.releaseAll();
}
```

### Long Press

Enable a second action when holding the key:

```cpp
#define LONG_PRESS_ENABLED   true
#define LONG_PRESS_MS        500               // hold threshold in ms
#define LONG_PRESS_KEY       KEY_MEDIA_PLAY_PAUSE
```

Short tap sends `KEY_TO_SEND`, holding past the threshold sends `LONG_PRESS_KEY`.

### Status LED

Wire an LED to a GPIO pin (see [HARDWARE.md](HARDWARE.md#step-5-add-a-status-led-optional)) and configure:

```cpp
#define LED_PIN   2              // GPIO pin (-1 to disable)
#define LED_MODE  LED_MODE_BOTH  // Blink when disconnected, flash on press
```

---

## 3D Printed Case

The parametric OpenSCAD source is in [`case/one_key_case.scad`](case/one_key_case.scad).

1. Open in [OpenSCAD](https://openscad.org) (free)
2. Measure your ESP32 board and update the dimensions
3. Export base and top plate as separate STL files
4. Print with 0.2mm layer height, 20% infill, no supports

See [HARDWARE.md — Step 6](HARDWARE.md#step-6-print-the-case) for detailed export and print instructions.

---

## Troubleshooting

| Problem | Solution |
|---------|----------|
| "OneKey" doesn't appear in Bluetooth | Check Serial Monitor for firmware output. Reset the ESP32. Ensure BLE is on. |
| Key press not detected | Verify wiring with a multimeter. Check that BUTTON_PIN matches your GPIO. |
| Keystroke not received by host | Remove pairing and re-pair. Try a different device. |
| Double-triggers | Increase `DEBOUNCE_MS` in config.h (try 80–100). |
| Upload fails | Hold BOOT button while uploading. Verify board and port selection. |

---

## Going Further

- **Battery power** — Add a LiPo battery + TP4056 charger for wireless operation
- **Multiple modes** — Long-press is already built in; add double-press detection for a third action
- **USB HID fallback** — ESP32-S3 supports TinyUSB for wired USB keyboard mode
- **Rotary encoder** — Add a volume knob alongside the key
- **RGB underglow** — WS2812B LED under a translucent case

---

*Built with the [ESP32-BLE-Keyboard](https://github.com/T-vK/ESP32-BLE-Keyboard) library by T-vK.*
