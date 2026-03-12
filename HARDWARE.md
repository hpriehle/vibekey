# Hardware Assembly — Step by Step

This guide walks you through every physical step of building the VibeKey 3-button keyboard, from bare parts to a working device. No prior electronics experience needed.

---

## What You Need on Your Workbench

Gather everything before you start.

### Parts

| # | Part | Qty | Notes |
|---|------|-----|-------|
| 1 | ESP32-S3 DevKitC-1 | 1 | Or any ESP32 dev board. The S3 is recommended — it has USB-C and better BLE. |
| 2 | MX-compatible key switch | 3 | Cherry MX, Gateron, or Kailh. Any MX-compatible switch will work. Pick your favorite feel. |
| 3 | MX-compatible keycap | 3 | 1u size (standard single key). |
| 4 | USB-C cable | 1 | For power and initial programming. |
| 5 | Wire, 22–26 AWG | ~18 inches total | You'll cut six ~3-inch pieces. Solid core is easier for breadboards; stranded is easier to solder. |
| 6 | (Optional) LED, 3mm or 5mm | 1 | Any color. For connection status feedback. |
| 7 | (Optional) 220Ω resistor | 1 | Current limiter for the LED. |

### Tools

| Tool | Required? | What It's For |
|------|-----------|---------------|
| Soldering iron + solder | Yes (for final build) | Connecting wires to the switch and board |
| Wire strippers | Yes | Stripping wire ends |
| Breadboard + jumper wires | Optional | Testing before soldering |
| Hot glue gun | Optional | Securing the board in the case |
| Heat-shrink tubing + lighter | Optional | Clean, insulated wire joints |
| 3D printer | Optional | Printing the case (or use any small box) |
| Multimeter | Optional | Debugging connections |

---

## Step 1: Understand the Key Switch

Before wiring anything, look at your MX switch.

```
        ┌──────────────────┐
        │    KEYCAP STEM   │  ← Keycap clips onto this cross-shaped stem
        │    ┌──┐  ┌──┐    │
        │    │  │  │  │    │
        └────┴──┴──┴──┴────┘
               SWITCH BODY
        ┌──────────────────┐
        │                  │
        │    (click leaf   │
        │     mechanism    │
        │     inside)      │
        │                  │
        └──┬────────────┬──┘
           │            │
         PIN A        PIN B      ← Two metal pins on the bottom
```

- MX switches have **two metal pins** on the bottom (some have additional plastic alignment pins — ignore those).
- When you press the key, the two metal pins are **connected** (closed circuit). When released, they're **disconnected** (open circuit).
- It doesn't matter which pin is "A" and which is "B" — there's no polarity.

---

## Step 2: The Wiring Plan

You have 3 switches and need 6 wires total. Each switch connects to its own GPIO pin and to GND. The GND wires can share a common ground — you can daisy-chain them (see below).

### Connection Table

| Switch   | Switch Pin A → | Switch Pin B → |
|----------|----------------|----------------|
| Button 1 | ESP32 **GPIO 4** | ESP32 **GND** |
| Button 2 | ESP32 **GPIO 5** | ESP32 **GND** |
| Button 3 | ESP32 **GPIO 6** | ESP32 **GND** |

### Wiring Diagram

```
   SWITCH 1          SWITCH 2          SWITCH 3
  ┌────────┐        ┌────────┐        ┌────────┐
  │  [KC]  │        │  [KC]  │        │  [KC]  │
  └──┬──┬──┘        └──┬──┬──┘        └──┬──┬──┘
     │  │              │  │              │  │
     │  │              │  │              │  │
     │  └──────────────┼──┴──────────────┼──┴──── GND (shared)
     │                 │                 │
     ▼                 ▼                 ▼
   GPIO 4           GPIO 5           GPIO 6

                  ┌──────────────────────┐
                  │      ESP32 Board     │
                  │     ┌──────────┐     │
                  │     │  USB-C   │     │
                  └─────┴──────────┴─────┘
```

**Tip — shared GND:** Instead of running 3 separate GND wires back to the ESP32, you can connect all three switch GND pins together with a single wire running between them (daisy-chain), then run one wire from that chain to any GND pin on the ESP32. This saves wires and keeps things tidy.

```
  Daisy-chain GND:
  Switch1-GND ────── Switch2-GND ────── Switch3-GND ────── ESP32 GND
```

---

## Step 3: Prototype on a Breadboard (Recommended)

Test everything before soldering.

### 3a. Insert the ESP32 into the breadboard

Place the ESP32 dev board straddling the center channel. Make sure the USB port is accessible.

### 3b. Wire the switches with jumper wires

For each of the 3 switches:
- Use a jumper wire from the switch's **pin A** to the corresponding **GPIO** (4, 5, or 6).
- Use a jumper wire from the switch's **pin B** to **GND**.

Switch pins may not fit directly into a breadboard. You can hold jumper wires against the switch pins for testing, or solder short leads onto the pins.

### 3c. Flash the firmware

Upload the firmware (see README for Arduino IDE or PlatformIO instructions). Open Serial Monitor at 115200 baud. You should see:

```
=============================
  VibeKey BLE Keyboard v3.0
=============================
Loaded mappings: MAP:226,234,233
Button 1 on GPIO 4 -> key code 226
Button 2 on GPIO 5 -> key code 234
Button 3 on GPIO 6 -> key code 233
LED on GPIO 2
Initializing BLE...
BLE advertising started.
Waiting for connection...
```

### 3d. Test each button

1. Pair "VibeKey" from your computer's Bluetooth settings.
2. Press each switch — Serial Monitor should print `Button X DOWN` and `-> sending key code`.
3. Test: Button 1 should mute, Button 2 should lower volume, Button 3 should raise volume.

If it works, you're ready to solder the final build.

---

## Step 4: Solder the Wires to the Switches

### 4a. Prepare the wires

1. Cut **six wires**, each about 3 inches (8 cm) long. Or four if you're daisy-chaining GND.
2. Strip about 5mm of insulation from each end.
3. (Optional) Slide heat-shrink tubing onto each wire before soldering.

### 4b. Tin and solder each switch

For each of the 3 switches:

1. **Tin the switch pins:** Touch the iron to a pin for 2 seconds, apply solder to create a small blob. Repeat for the other pin.
2. **Solder the signal wire:** Hold a stripped wire end against **pin A**. Touch the iron to both wire and pin. The solder reflows and bonds. Hold still 3 seconds to cool.
3. **Solder the GND wire:** Same technique on **pin B**.
4. **Tug test:** Gently pull each wire to confirm a solid joint.
5. **Insulate:** Slide heat-shrink over joints and apply heat.

If daisy-chaining GND: solder a short wire from Switch 1's pin B to Switch 2's pin B, then from Switch 2's pin B to Switch 3's pin B, then one longer wire from any of those to ESP32 GND.

---

## Step 5: Solder the Wires to the ESP32

### 5a. Identify the pins

Find these pins on your ESP32 board (labeled on the silk screen):

| Wire From | ESP32 Pin |
|-----------|-----------|
| Switch 1 signal | **GPIO 4** (labeled "4" or "IO4") |
| Switch 2 signal | **GPIO 5** (labeled "5" or "IO5") |
| Switch 3 signal | **GPIO 6** (labeled "6" or "IO6") |
| GND (shared or individual) | **GND** (any GND pin) |

### 5b. Attach the wires

**Option A — Solder to header pins (recommended):**
Wrap the stripped wire around the header pin and solder.

**Option B — Through-hole solder:**
Push wire through the board's through-hole and solder on the bottom.

**Option C — DuPont connectors (no soldering):**
Crimp or solder your wires to female DuPont connectors and plug onto the header pins. Fully reversible.

---

## Step 5: Add a Status LED (Optional)

An LED gives visual feedback: slow blink = searching for BLE connection, flash = keypress registered.

### Circuit

```
  ESP32 GPIO 2 ──── [220Ω resistor] ──── LED (+/longer leg) ──── LED (-/shorter leg) ──── GND
```

### Solder it

1. Solder the 220Ω resistor to GPIO 2.
2. Solder the other resistor lead to the LED's **longer leg** (anode, +).
3. Solder a wire from the LED's **shorter leg** (cathode, -) to GND.
4. Insulate joints with heat-shrink or electrical tape.

### Configure

In `firmware/config.h`, set `LED_PIN` to match your wiring:
```cpp
#define LED_PIN  2    // change to your GPIO, or -1 to disable
```

---

## Step 6: Print or Build a Case

The OpenSCAD source in `case/one_key_case.scad` was designed for a single key. For 3 keys, you'll need to modify it:

1. Open in [OpenSCAD](https://openscad.org) (free).
2. Widen the top plate to fit **three 14mm x 14mm cutouts** side by side, with ~2mm spacing between them.
3. Widen the base to match.
4. Measure your ESP32 board and update dimensions.
5. Export base and top plate as STL files.

### Print Settings

| Setting | Value |
|---------|-------|
| Layer height | 0.2mm |
| Infill | 20% |
| Supports | None needed |
| Material | PLA or PETG |

### No 3D printer?

- Use any small plastic box — cut three 14mm square holes in the top.
- A piece of thick cardboard works for prototyping.
- Order from an online 3D printing service.

---

## Step 7: Final Assembly

### 7a. Insert switches into the top plate

1. Orient each switch with pins facing down.
2. Align with the 14mm cutouts and push firmly until clips snap.
3. Apply a dab of hot glue if any switch is loose.

```
  TOP PLATE (top view)
  ┌──────────────────────────────────────────┐
  │   ┌────────┐  ┌────────┐  ┌────────┐    │
  │   │ SWITCH │  │ SWITCH │  │ SWITCH │    │
  │   │   1    │  │   2    │  │   3    │    │
  │   └────────┘  └────────┘  └────────┘    │
  └──────────────────────────────────────────┘
```

### 7b. Mount the ESP32 in the base

1. Place the ESP32 in the base with USB port aligned to the wall cutout.
2. Secure with hot glue on two corners or double-sided tape.

### 7c. Route the wires

1. Connect switch wires to the ESP32 pins (GPIO 4, 5, 6, and GND).
2. Fold wires neatly — no bare wire should touch anything conductive.

### 7d. Close the case and add keycaps

1. Set the top plate onto the base.
2. Press each keycap onto its switch stem.

---

## Step 8: Power On, Pair, and Remap

### First boot

1. Plug USB-C into the ESP32.
2. LED slow-blinks (searching for connection).
3. Go to Bluetooth settings → pair **"VibeKey"** → no PIN needed.
4. Press each button to test (defaults: mute, vol down, vol up).

### Remap your buttons

With the ESP32 still plugged into USB:

```bash
pip install pyserial          # one-time
python3 mapper/mapper.py      # opens interactive menu
```

Type `1`, `2`, or `3` to change a button. Type `k` to see all available keys. Your new mappings are saved on the ESP32 and persist across reboots.

Or remap from the command line:

```bash
python3 mapper/mapper.py --set mute play_pause f13
```

---

## Troubleshooting

| Symptom | Check |
|---------|-------|
| LED doesn't light up | Verify LED_PIN in config.h. Check LED polarity (long leg = +). Check resistor. |
| "VibeKey" not in Bluetooth | Check Serial Monitor. Reset ESP32. Make sure BLE is on. |
| One button not working | Check that specific GPIO wiring. Use Serial Monitor to see if press is detected. Swap to a known-working GPIO to isolate. |
| Key fires but wrong action | Use the mapper tool to check/change mappings: `python3 mapper/mapper.py --get` |
| Double-triggers | Increase DEBOUNCE_MS in config.h (try 80–100). |
| Mapper can't find serial port | Use `python3 mapper/mapper.py --list-ports` to find your port, then `--port /dev/ttyUSB0`. |
| Upload fails | Hold BOOT button while uploading. Verify board and port in Arduino IDE. |

---

## Finished!

```
  ┌──────────────────────────────────────────┐
  │   ┌──────┐   ┌──────┐   ┌──────┐        │
  │   │ KC 1 │   │ KC 2 │   │ KC 3 │        │
  │   │ MUTE │   │ VOL- │   │ VOL+ │        │
  │   └──────┘   └──────┘   └──────┘        │  ← Top plate
  │══════════════════════════════════════════│
  │  ┌────────────────────────────────────┐  │
  │  │          ESP32 + wires             │  │  ← Base
  │  └────────────────────────────────────┘  │
  └──────────────────┤USB-C├─────────────────┘
```

To change what any button does, run `python3 mapper/mapper.py` — no reflashing needed.
