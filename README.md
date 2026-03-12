# VibeKey — 3-Button Bluetooth Keyboard

**ESP32-Powered · 3D Printed · BLE HID · Remappable**

Difficulty: Beginner | Build Time: 2–4 hours | Cost: ~$18

---

## What Is This?

Three mechanical keys that connect to your computer, phone, or tablet over Bluetooth. Each button sends whatever keystroke you configure — mute, volume up/down, media controls, push-to-talk, or any key you want. Remap the buttons any time using the included Python tool. No reflashing needed.

| Detail          | Spec                                                         |
|-----------------|--------------------------------------------------------------|
| Microcontroller | ESP32-S3 DevKitC-1 (recommended) or any ESP32 dev board     |
| Connectivity    | Bluetooth Low Energy (BLE) HID — no drivers needed           |
| Power           | USB-C (bus-powered from your computer or a power bank)       |
| Key Switches    | 3x MX-compatible mechanical switches (Cherry, Gateron, Kailh)|
| Firmware        | Arduino / PlatformIO + ESP32-BLE-Keyboard library            |
| Key Mapper      | Python CLI tool over USB serial — remap without reflashing   |
| Case            | 3D-printed enclosure (OpenSCAD parametric source included)   |

---

## Project Structure

```
vibekey/
├── README.md                       # This file
├── HARDWARE.md                     # Step-by-step hardware assembly guide
├── firmware/
│   ├── one_key_keyboard.ino        # Main firmware (Arduino sketch)
│   └── config.h                    # Pin assignments, BLE name, LED config
├── mapper/
│   ├── mapper.py                   # Python key mapper tool
│   └── requirements.txt            # Python dependencies (pyserial)
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
- 3x MX-compatible key switches + keycaps (~$5–6)
- USB-C cable
- Six short wires (22–26 AWG)
- (Optional) LED + 220Ω resistor for status indicator

### 2. Wire It Up

Three switches, six wires. Each switch connects one pin to a GPIO and the other to GND.

| Switch   | Pin A → ESP32 GPIO | Pin B → ESP32 |
|----------|---------------------|---------------|
| Button 1 | GPIO 4              | GND           |
| Button 2 | GPIO 5              | GND           |
| Button 3 | GPIO 6              | GND           |

For the full step-by-step with diagrams, soldering instructions, LED wiring, case assembly, and troubleshooting, see **[HARDWARE.md](HARDWARE.md)**.

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
3. Pair **"VibeKey"** — no PIN required
4. Press the buttons!

Default mappings: Button 1 = Mute, Button 2 = Volume Down, Button 3 = Volume Up.

---

## Remapping Keys

The Python mapper tool lets you change what each button does — no reflashing, no code editing. Mappings are saved on the ESP32 and persist across reboots.

### Install

```bash
pip install pyserial
```

### Interactive Mode

```bash
python3 mapper/mapper.py
```

This opens an interactive menu:

```
╔══════════════════════════════════╗
║      VibeKey Key Mapper          ║
╚══════════════════════════════════╝

  Current mappings:
  ─────────────────────────────
  Button 1  →  mute  (code 226)
  Button 2  →  volume_down  (code 234)
  Button 3  →  volume_up  (code 233)

  Commands:
  [1/2/3]  Change button 1, 2, or 3
  [k]      Show all available key names
  [q]      Quit
```

Type `1`, `2`, or `3` to remap a button. Type `k` to see all available key names.

### Command-Line Mode

```bash
# Show current mappings
python3 mapper/mapper.py --get

# Set all 3 buttons at once
python3 mapper/mapper.py --set mute play_pause f13

# Specify serial port manually
python3 mapper/mapper.py --port /dev/ttyUSB0

# List available serial ports
python3 mapper/mapper.py --list-ports
```

### Available Keys

| Category | Keys |
|----------|------|
| Media    | `mute` `volume_up` `volume_down` `play_pause` `next_track` `prev_track` `stop` |
| Common   | `enter` `esc` `backspace` `tab` `space` `delete` `insert` `home` `end` `page_up` `page_down` |
| Arrows   | `up` `down` `left` `right` |
| F-Keys   | `f1` through `f24` |
| Letters  | `a` through `z` |
| Numbers  | `0` through `9` |
| Modifiers| `left_ctrl` `left_shift` `left_alt` `left_gui` `right_ctrl` `right_shift` `right_alt` `right_gui` |

---

## Configuration

Pin assignments, BLE name, LED settings, and default key mappings are in [`firmware/config.h`](firmware/config.h). These require a reflash to change.

### Pin Assignments

```cpp
#define BUTTON_PIN_1    4    // GPIO for button 1
#define BUTTON_PIN_2    5    // GPIO for button 2
#define BUTTON_PIN_3    6    // GPIO for button 3
#define LED_PIN         2    // GPIO for status LED (-1 to disable)
```

### Status LED

Wire an LED to a GPIO pin (see [HARDWARE.md](HARDWARE.md#step-5-add-a-status-led-optional)) and configure:

```cpp
#define LED_PIN   2              // GPIO pin (-1 to disable)
#define LED_MODE  LED_MODE_BOTH  // Blink when disconnected, flash on press
```

---

## 3D Printed Case

The parametric OpenSCAD source is in [`case/one_key_case.scad`](case/one_key_case.scad). You'll need to modify it for 3 switches (widen the top plate to fit three 14mm cutouts).

1. Open in [OpenSCAD](https://openscad.org) (free)
2. Measure your ESP32 board and update the dimensions
3. Export base and top plate as separate STL files
4. Print with 0.2mm layer height, 20% infill, no supports

See [HARDWARE.md — Step 6](HARDWARE.md#step-6-print-or-build-a-case) for detailed instructions.

---

## Troubleshooting

| Problem | Solution |
|---------|----------|
| "VibeKey" doesn't appear in Bluetooth | Check Serial Monitor for firmware output. Reset the ESP32. Ensure BLE is on. |
| Button press not detected | Verify wiring with a multimeter. Check that BUTTON_PIN_x matches your GPIO. |
| Keystroke not received by host | Remove pairing and re-pair. Try a different device. |
| Double-triggers | Increase `DEBOUNCE_MS` in config.h (try 80–100). |
| Mapper can't find serial port | Use `--list-ports` to see available ports, then specify with `--port`. |
| Upload fails | Hold BOOT button while uploading. Verify board and port selection. |

---

*Built with the [ESP32-BLE-Keyboard](https://github.com/T-vK/ESP32-BLE-Keyboard) library by T-vK.*
