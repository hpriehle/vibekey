# 1-Key Bluetooth Keyboard — Full Build Plan

**ESP32-Powered · 3D Printed · BLE HID**

Difficulty: Beginner | Build Time: 2–4 hours | Cost: ~$15

---

## Project Overview

This guide walks you through building a single-key Bluetooth keyboard powered by an ESP32 microcontroller. When complete, your computer, phone, or tablet will see it as a standard Bluetooth keyboard. Press the key and it sends whatever keystroke or macro you configure — a mute button, a media key, a push-to-talk trigger, a stream deck button, or anything else you can imagine.

| Detail          | Spec                                                         |
|-----------------|--------------------------------------------------------------|
| Microcontroller | ESP32-S3 DevKitC-1 (recommended) or any ESP32 dev board     |
| Connectivity    | Bluetooth Low Energy (BLE) HID — no drivers needed           |
| Power           | USB-C (bus-powered from your computer or a power bank)       |
| Key Switch      | Any MX-compatible mechanical switch (Cherry, Gateron, Kailh) |
| Firmware        | Arduino IDE + ESP32-BLE-Keyboard library by T-vK             |
| Case            | 3D-printed enclosure (OpenSCAD source included)              |

---

## Parts & Tools

### Required Parts

- **ESP32 dev board** — ESP32-S3 DevKitC-1 preferred (~$8–10 on AliExpress/Amazon). Any ESP32 works for BLE, but the S3 also supports native USB HID.
- **Mechanical key switch** — Cherry MX, Gateron, or Kailh. Any MX-compatible switch. (~$1–2)
- **Keycap** — Any MX-compatible keycap. (~$1–2)
- **USB-C cable** — For programming and power.
- **Two short wires** — ~3 inches each, for connecting the switch to the board. 22–26 AWG solid core or stranded.

### Optional But Recommended

- **Breadboard** — For prototyping before soldering.
- **Soldering iron + solder** — For the final build.
- **Hot glue gun** — To secure the board inside the case.
- **Heat-shrink tubing** — For clean wire connections.

### Tools

- 3D printer (any FDM printer — PLA is fine)
- Computer with Arduino IDE installed
- Small Phillips screwdriver (optional, if your case uses screws)

---

## Phase 1: Software Setup

Get your development environment ready before touching any hardware.

### Step 1 — Install Arduino IDE

- Download Arduino IDE 2.x from [arduino.cc/en/software](https://www.arduino.cc/en/software)
- Install it and open it

### Step 2 — Add ESP32 Board Support

- Go to **File → Preferences**
- In "Additional Boards Manager URLs", add:

```
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
```

- Go to **Tools → Board → Boards Manager**, search "esp32", install the package by Espressif Systems
- Select your board under **Tools → Board → ESP32 Arduino → "ESP32S3 Dev Module"** (or your specific board)

### Step 3 — Install the BLE Keyboard Library

- Go to [github.com/T-vK/ESP32-BLE-Keyboard/releases](https://github.com/T-vK/ESP32-BLE-Keyboard/releases)
- Download the latest `.zip` release
- In Arduino IDE: **Sketch → Include Library → Add .ZIP Library** → select the downloaded zip

---

## Phase 2: Wiring

This is the simplest wiring you'll ever do. A mechanical switch is just two metal contacts — when you press the key, they connect.

### Connections

Only two wires are needed:

| Wire   | From (Switch Pin)    | To (ESP32 Pin) |
|--------|----------------------|-----------------|
| Wire 1 | Either pin on switch | GPIO 4          |
| Wire 2 | Other pin on switch  | GND             |

The firmware enables the internal pull-up resistor on GPIO 4, so when the switch is open the pin reads HIGH, and when pressed it connects to GND and reads LOW. No external resistor needed.

### Wiring Diagram (ASCII)

```
                 ┌─────────────┐
                 │  MECHANICAL  │
                 │   SWITCH     │
                 │              │
                 └──┬───────┬──┘
                    │       │
                    │       │
          Wire 1   │       │  Wire 2
                    │       │
                    ▼       ▼
               ┌─────┐  ┌─────┐
               │GPIO4│  │ GND │
               └──┬──┘  └──┬──┘
                  │         │
            ┌─────┴─────────┴─────┐
            │                     │
            │     ESP32 Board     │
            │                     │
            │    ┌───────────┐    │
            │    │  USB-C    │    │
            └────┴───────────┴────┘
```

For prototyping, use a breadboard and jumper wires. For the final build, solder wires directly to the switch pins and the ESP32 header pins.

---

## Phase 3: Firmware

The complete firmware is in [`firmware/one_key_keyboard.ino`](firmware/one_key_keyboard.ino). Open it in Arduino IDE.

### Uploading the Firmware

1. Connect your ESP32 via USB
2. Select the correct board and port under **Tools**
3. If using an ESP32-S3: set **USB CDC On Boot** to "Enabled" in Tools
4. Click **Upload** (arrow button)
5. Open **Serial Monitor** (115200 baud) to see debug messages

If the upload fails, hold the **BOOT** button on the ESP32 while clicking Upload, then release BOOT after the upload starts.

### Customizing the Key

Change `KEY_TO_SEND` in the firmware to any key you want. Common options:

| Constant                | Use Case                                                   |
|-------------------------|------------------------------------------------------------|
| `KEY_MEDIA_MUTE`        | Mute/unmute microphone (Zoom, Teams, etc.)                 |
| `KEY_MEDIA_PLAY_PAUSE`  | Play/pause music or video                                  |
| `KEY_MEDIA_VOLUME_UP`   | Volume up                                                  |
| `KEY_MEDIA_VOLUME_DOWN` | Volume down                                                |
| `KEY_MEDIA_NEXT_TRACK`  | Next song/track                                            |
| `KEY_F13`               | F13 — perfect for push-to-talk (not on standard keyboards) |
| `KEY_RETURN`            | Enter key                                                  |
| `KEY_DELETE`            | Delete key                                                 |
| `'a'` (or any letter)  | Sends a single character                                   |

### Sending Key Combos

For modifier combos (like Ctrl+Shift+M for Zoom mute), set `USE_COMBO = true` and edit the `sendCombo()` function in the firmware:

```cpp
void sendCombo() {
  bleKeyboard.press(KEY_LEFT_CTRL);
  bleKeyboard.press(KEY_LEFT_SHIFT);
  bleKeyboard.press('m');
  delay(50);
  bleKeyboard.releaseAll();
}
```

---

## Phase 4: 3D Printed Case

The case is a simple two-part box: a base that holds the ESP32 dev board, and a top plate with a 14mm x 14mm cutout for the MX switch. The switch snaps into the top plate, and the top plate sits on the base.

The OpenSCAD source is in [`case/one_key_case.scad`](case/one_key_case.scad). Open it in [OpenSCAD](https://openscad.org) (free), adjust dimensions to your board, and export as STL.

### Design Specs

- **Switch cutout:** 14mm x 14mm square (MX standard)
- **Board clearance:** Sized for a typical ESP32 DevKit (~52mm x 28mm). Measure your specific board.
- **USB port:** Cutout on one side for the USB-C connector
- **Wall thickness:** 2mm
- **Material:** PLA, PETG, or anything your printer handles

### Print Settings

- Layer height: 0.2mm
- Infill: 20% is plenty
- Supports: Not needed for this simple geometry
- Orientation: Print the base upright, print the top plate face-down

### Assembly Tips

- The switch should snap firmly into the 14mm cutout in the top plate. If it's loose, add a tiny dab of hot glue.
- Secure the ESP32 board in the base with hot glue or double-sided tape. Make sure the USB port lines up with the cutout.
- Route the two wires from the switch through the inside of the case to the ESP32 pins.
- The top plate sits on the base walls. You can friction-fit it, glue it, or add screw posts in OpenSCAD if you want it to be removable.

---

## Phase 5: Pair & Test

1. Plug the ESP32 into USB for power
2. Wait 2–3 seconds for it to boot and start advertising
3. On your computer/phone, go to Bluetooth settings and look for **"OneKey"**
4. Pair it — it should connect as a keyboard with no PIN required
5. Press the key — you should see the configured keystroke fire

Open a text editor to test letter keys, or try the mute key on a Zoom call.

### Troubleshooting

- **"OneKey" doesn't appear in Bluetooth:** Make sure the firmware uploaded successfully (check Serial Monitor). Try resetting the ESP32. Some boards need USB CDC On Boot enabled.
- **Pairs but key doesn't work:** Verify wiring — use Serial Monitor to confirm "Key pressed!" messages appear. If they do, the issue is on the Bluetooth side — try removing and re-pairing.
- **Key fires multiple times:** Increase `DEBOUNCE_MS` in the firmware (try 80 or 100).
- **Upload fails:** Hold the BOOT button while uploading. Make sure the correct board and port are selected.
- **Reconnection issues:** On some platforms, you may need to remove the pairing and re-pair after reflashing.

---

## Going Further

Once you have the basic build working, here are some ideas to level it up:

- **Add an LED:** Wire an LED to another GPIO pin and light it up when connected or on keypress.
- **Battery power:** Add a LiPo battery and TP4056 charger module for a fully wireless build.
- **Multiple modes:** Use a long-press or double-press to switch between different keystrokes (mute on single press, play/pause on double press).
- **USB HID fallback:** If using an ESP32-S3, add TinyUSB support so it works as a USB keyboard when plugged in and Bluetooth when not.
- **Rotary encoder:** Add a rotary encoder next to the key for volume control.
- **RGB underglow:** Add a WS2812B LED under a translucent case for style points.

---

## Project Structure

```
vibekey/
├── README.md                      # This file
├── firmware/
│   └── one_key_keyboard.ino       # Arduino sketch (complete firmware)
└── case/
    └── one_key_case.scad          # OpenSCAD 3D model for the enclosure
```

---

*Built with an ESP32 and the [ESP32-BLE-Keyboard](https://github.com/T-vK/ESP32-BLE-Keyboard) library by T-vK.*
