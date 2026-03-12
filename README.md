# VibeKey — 3-Button Bluetooth Keyboard

**ESP32-Powered · 3D Printed · BLE HID · Remappable via CLI**

Difficulty: Beginner | Build Time: 2–4 hours | Cost: ~$18

---

## What Is This?

Three mechanical keys that connect to your computer, phone, or tablet over Bluetooth. Each button sends whatever keystroke you configure — mute, volume up/down, media controls, push-to-talk, or any key you want. Remap the buttons any time from the command line. No reflashing needed.

| Detail          | Spec                                                         |
|-----------------|--------------------------------------------------------------|
| Microcontroller | ESP32-S3 DevKitC-1 (recommended) or any ESP32 dev board     |
| Connectivity    | Bluetooth Low Energy (BLE) HID — no drivers needed           |
| Power           | USB-C (bus-powered from your computer or a power bank)       |
| Key Switches    | 3x MX-compatible mechanical switches (Cherry, Gateron, Kailh)|
| Firmware        | Arduino / PlatformIO + ESP32-BLE-Keyboard library            |
| CLI Tool        | `vibekey` — setup, flash, remap, monitor, all in one         |
| Case            | 3D-printed enclosure (OpenSCAD parametric source included)   |

---

## Quick Start

```bash
./vibekey setup
```

That's it. The setup wizard walks you through everything:
1. Installs dependencies (PlatformIO, pyserial)
2. Configures your GPIO pins and Bluetooth name
3. Builds and flashes the firmware
4. Helps you pair over Bluetooth
5. Maps your 3 buttons interactively

---

## The `vibekey` CLI

One tool to manage everything. Run `./vibekey help` for the full reference.

### Setup (first time)

```bash
./vibekey setup             # guided wizard — installs tools, flashes, maps keys
```

### Remap buttons

```bash
./vibekey map               # interactive menu
./vibekey map mute f13 space        # set all 3 from command line
./vibekey map --get         # show current mappings
```

### Device management

```bash
./vibekey status            # show device info and current mappings
./vibekey monitor           # live serial output (debug)
./vibekey ports             # list available serial ports
./vibekey reset             # reset to default mappings (mute, vol_down, vol_up)
./vibekey keys              # show all available key names
```

### Flash firmware

```bash
./vibekey flash             # build and upload (ESP32-S3)
./vibekey flash --env esp32         # use original ESP32 board
./vibekey flash --build-only        # compile without uploading
./vibekey flash --monitor   # flash then open serial monitor
```

### Global options

```bash
./vibekey map -p /dev/ttyUSB0       # specify serial port (auto-detected by default)
```

---

## Available Keys

Use any of these names with `vibekey map`:

| Category | Keys |
|----------|------|
| Media    | `mute` `volume_up` `volume_down` `play_pause` `next_track` `prev_track` `stop` |
| Common   | `enter` `esc` `backspace` `tab` `space` `delete` `insert` `home` `end` `page_up` `page_down` |
| Arrows   | `up` `down` `left` `right` |
| F-Keys   | `f1` through `f24` |
| Letters  | `a` through `z` |
| Numbers  | `0` through `9` |
| Modifiers| `left_ctrl` `left_shift` `left_alt` `left_gui` `right_ctrl` `right_shift` `right_alt` `right_gui` |

Run `./vibekey keys` to see this list any time.

---

## Project Structure

```
vibekey/
├── vibekey                         # Main CLI tool (setup, flash, map, monitor)
├── README.md                       # This file
├── HARDWARE.md                     # Step-by-step hardware assembly guide
├── firmware/
│   ├── one_key_keyboard.ino        # Main firmware (Arduino sketch)
│   └── config.h                    # Pin assignments, BLE name, LED config
├── mapper/
│   ├── mapper.py                   # Standalone key mapper (also usable directly)
│   └── requirements.txt            # Python dependencies (pyserial)
├── case/
│   └── one_key_case.scad           # OpenSCAD parametric 3D model
├── platformio.ini                  # PlatformIO build configuration
├── flash.sh                        # Legacy flash script
└── .gitignore
```

---

## Hardware

### Parts needed

- ESP32-S3 DevKitC-1 (~$8–10)
- 3x MX-compatible key switches + keycaps (~$5–6)
- USB-C cable
- Six short wires (22–26 AWG)
- (Optional) LED + 220Ω resistor for status indicator

### Wiring

| Switch   | Pin A → ESP32 GPIO | Pin B → ESP32 |
|----------|---------------------|---------------|
| Button 1 | GPIO 4              | GND           |
| Button 2 | GPIO 5              | GND           |
| Button 3 | GPIO 6              | GND           |

For the complete assembly walkthrough with diagrams, soldering tips, LED wiring, case printing, and troubleshooting, see **[HARDWARE.md](HARDWARE.md)**.

---

## Configuration

Pin assignments, BLE name, and LED settings live in [`firmware/config.h`](firmware/config.h). These require a reflash (`vibekey flash`) to change.

Key mappings are stored on the ESP32's flash memory and changed at runtime via `vibekey map` — no reflashing needed.

```cpp
// firmware/config.h
#define BUTTON_PIN_1    4         // GPIO for button 1
#define BUTTON_PIN_2    5         // GPIO for button 2
#define BUTTON_PIN_3    6         // GPIO for button 3
#define LED_PIN         2         // GPIO for status LED (-1 to disable)
#define BLE_DEVICE_NAME "VibeKey" // Bluetooth name (max 15 chars)
```

The setup wizard (`vibekey setup`) can configure all of these interactively.

---

## Troubleshooting

| Problem | Solution |
|---------|----------|
| "VibeKey" doesn't appear in Bluetooth | `vibekey monitor` to check firmware output. Reset ESP32. |
| Button press not detected | Check wiring. Run `vibekey monitor` to see debug output. |
| Keystroke not received by host | Remove Bluetooth pairing, re-pair. Try different device. |
| Double-triggers | Increase `DEBOUNCE_MS` in config.h (try 80–100), reflash. |
| Mapper can't find serial port | `vibekey ports` to see available ports, then use `-p PORT`. |
| Upload fails | Hold BOOT button while uploading. Check board/port settings. |
| Wrong key sent | `vibekey map --get` to check current mappings, remap with `vibekey map`. |

---

*Built with the [ESP32-BLE-Keyboard](https://github.com/T-vK/ESP32-BLE-Keyboard) library by T-vK.*
