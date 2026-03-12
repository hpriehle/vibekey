# Hardware Assembly — Step by Step

This guide walks you through every physical step of building the OneKey keyboard, from bare parts to a working device. No prior electronics experience needed.

---

## What You Need on Your Workbench

Gather everything before you start. There's nothing worse than being mid-solder and realizing you're missing a wire.

### Parts

| # | Part | Notes |
|---|------|-------|
| 1 | ESP32-S3 DevKitC-1 | Or any ESP32 dev board. The S3 is recommended — it has USB-C and better BLE. |
| 2 | MX-compatible key switch | Cherry MX, Gateron, Kailh — any will work. Pick your favorite feel. |
| 3 | MX-compatible keycap | 1u size (standard single key). |
| 4 | USB-C cable | For power and initial programming. |
| 5 | Two wires, ~3 inches each | 22–26 AWG. Solid core is easier for breadboards; stranded is easier to solder. |
| 6 | (Optional) LED, 3mm or 5mm | Any color. For connection status feedback. |
| 7 | (Optional) 220Ω resistor | Current limiter for the LED. |

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

Before wiring anything, take a moment to look at your MX switch.

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
- It doesn't matter which pin is "A" and which is "B" — a switch is just a momentary contact. There's no polarity.

---

## Step 2: Prototype on a Breadboard (Optional but Recommended)

If you have a breadboard, test everything before soldering. This lets you verify the firmware works before committing to permanent connections.

### 2a. Insert the ESP32 into the breadboard

- Place the ESP32 dev board straddling the center channel of the breadboard.
- The pins on each side should sit in the breadboard holes.
- Make sure the USB port is accessible and not blocked.

```
     ┌──────────────────────────────────┐
     │  o o o o o o o o o o o o o o o o │  ← breadboard row
     │  ┌─────────────────────────────┐ │
     │  │         ESP32 BOARD         │ │
     │  │  [3V3] [GND] [GPIO4] ...   │ │
     │  └─────────────────────────────┘ │
     │  o o o o o o o o o o o o o o o o │  ← breadboard row
     └──────────────────────────────────┘
```

### 2b. Wire the switch with jumper wires

- Use a male-to-male jumper wire from **GPIO 4** on the ESP32 to **one pin** of the switch.
- Use another jumper wire from **GND** on the ESP32 to the **other pin** of the switch.
- The switch pins may not fit directly into a breadboard. You can either:
  - Hold the jumper wires against the switch pins while testing, or
  - Solder short leads onto the switch pins, then plug those into the breadboard.

### 2c. Flash the firmware

- Follow the software setup in the main README (Arduino IDE or PlatformIO).
- Upload the firmware to the ESP32.
- Open the Serial Monitor at 115200 baud.
- You should see:
  ```
  =============================
    OneKey BLE Keyboard v2.0
  =============================
  Button on GPIO 4
  Initializing BLE...
  BLE advertising started.
  Waiting for connection...
  ```

### 2d. Test the button

- Pair "OneKey" from your computer or phone's Bluetooth settings.
- Press the switch — the Serial Monitor should print `Key DOWN` and `-> sending keystroke`.
- Open a text editor and verify the keystroke arrives.

If this all works, you're ready to solder the final build!

---

## Step 3: Solder the Wires to the Switch

Now we'll make permanent connections.

### 3a. Prepare the wires

1. Cut two wires, each about **3 inches (8 cm)** long. A bit of extra length is fine — you can trim later.
2. Strip about **5mm (3/16 inch)** of insulation from each end of each wire.
3. (Optional) Slide a piece of heat-shrink tubing onto each wire now, before soldering. You'll shrink it later to cover the joint.

### 3b. Tin the switch pins

1. Heat your soldering iron and let it reach temperature (~350°C / 660°F for leaded solder).
2. Hold the switch with the pins facing up (or use helping hands / a vice).
3. Touch the iron tip to one switch pin for 2 seconds, then touch solder to the pin (not the iron). A small blob of solder should coat the pin.
4. Repeat for the other pin.

```
  SWITCH (bottom view)
  ┌──────────────┐
  │              │
  │  (pin A) ●───── tiny solder blob
  │              │
  │  (pin B) ●───── tiny solder blob
  │              │
  └──────────────┘
```

### 3c. Solder the wires

1. Hold a stripped wire end against **pin A** of the switch.
2. Touch the iron to both the wire and the pin simultaneously for 2 seconds. The solder on the pin should reflow and bond to the wire.
3. Remove the iron and hold still for 3 seconds while it cools.
4. Repeat for the second wire on **pin B**.
5. Tug gently on each wire to confirm a solid joint.

### 3d. Insulate (optional)

- Slide the heat-shrink tubing over each solder joint and apply heat (lighter or heat gun) to shrink it snug.

---

## Step 4: Solder the Wires to the ESP32

### 4a. Identify the pins

Find these two pins on your ESP32 board. They're labeled on the silk screen (the white text printed on the board).

| Wire (from switch) | ESP32 Pin | Location |
|---------------------|-----------|----------|
| Wire from pin A | **GPIO 4** (labeled "4" or "IO4") | Varies by board — check your board's pinout diagram |
| Wire from pin B | **GND** | Usually multiple GND pins — use any one |

Most ESP32-S3 DevKitC-1 boards have GPIO 4 and GND on the same side, close together.

### 4b. Solder or use headers

**Option A — Solder directly to header pins (recommended):**
1. If your ESP32 has header pins already soldered on, wrap the stripped wire end around the header pin and solder it.
2. This is the most secure connection.

**Option B — Solder to through-holes:**
1. If your board has bare through-holes (no header pins), push the wire through from the top and solder on the bottom.

**Option C — Use DuPont connectors (no soldering):**
1. If you have female DuPont jumper wires, crimp or solder your switch wires to DuPont connectors and plug them onto the header pins.
2. Less secure but fully reversible.

```
  ESP32 BOARD (top view)
  ┌─────────────────────────┐
  │                         │
  │  ● ● ● ● ● ● ● ● ● ●  │ ← header pins
  │            ↑   ↑        │
  │          IO4  GND       │
  │            │   │        │
  │            │   └─── Wire from switch pin B
  │            └─────── Wire from switch pin A
  │                         │
  │      ┌──────────┐      │
  │      │  USB-C   │      │
  └──────┴──────────┴──────┘
```

---

## Step 5: Add a Status LED (Optional)

An LED gives you visual feedback: blinking when searching for a Bluetooth connection, flashing on keypress.

### 5a. Circuit

```
  ESP32 GPIO 2 ──── [220Ω resistor] ──── LED (+/longer leg) ──── LED (-/shorter leg) ──── GND
```

| Wire | From | To |
|------|------|----|
| Resistor lead 1 | GPIO 2 (or your chosen LED_PIN) | Resistor |
| Resistor lead 2 | Resistor | LED anode (longer leg, +) |
| LED cathode | LED shorter leg (-) | GND |

### 5b. Solder it

1. Solder the 220Ω resistor to the GPIO 2 header pin (or use the same technique as the switch wires).
2. Solder the other end of the resistor to the **longer leg** (anode, +) of the LED.
3. Solder a wire from the **shorter leg** (cathode, -) of the LED to GND.
4. Insulate exposed joints with heat-shrink or electrical tape.

### 5c. Configure in firmware

In `firmware/config.h`, make sure `LED_PIN` matches the GPIO you wired:

```cpp
#define LED_PIN  2    // GPIO 2 — change if you used a different pin
```

If you skip the LED, set `LED_PIN` to `-1` and no LED code will run.

---

## Step 6: Print the Case

### 6a. Export STL files from OpenSCAD

1. Download and install [OpenSCAD](https://openscad.org) (free).
2. Open `case/one_key_case.scad`.
3. **Measure your ESP32 board** with calipers or a ruler and update the dimensions at the top of the file:
   ```
   board_width  = 28;    // your board's width in mm
   board_length = 52;    // your board's length in mm
   board_height = 12;    // tallest component on the board
   ```
4. Export the **base**:
   - Comment out the `top_plate();` line (add `//` in front).
   - Press F6 to render, then **File → Export as STL**. Save as `base.stl`.
5. Export the **top plate**:
   - Uncomment `top_plate();` and comment out `base();`.
   - Press F6 to render, then export as `top_plate.stl`.

### 6b. Slice and print

| Setting | Value |
|---------|-------|
| Layer height | 0.2mm |
| Infill | 20% |
| Supports | None needed |
| Material | PLA or PETG |
| Base orientation | Upright (open side up) |
| Top plate orientation | Flipped (flat top facing the bed) |

Print time is roughly 30–60 minutes for both parts.

### 6c. No 3D printer?

Alternatives:
- Use any small plastic box or enclosure — drill/cut a 14mm square hole in the top.
- Use a thick piece of cardboard as a prototype case.
- Order from an online 3D printing service.

---

## Step 7: Final Assembly

### 7a. Insert the switch into the top plate

1. Orient the switch so the pins face downward.
2. Align the switch body with the 14mm square cutout in the top plate.
3. Push firmly until the switch clips snap into place. You should feel and hear a click.
4. The switch should be held securely. If it's loose, apply a small dab of hot glue on the underside.

```
  TOP PLATE (cross section)
  ┌───────┬──────────┬───────┐
  │       │  SWITCH  │       │  ← switch clips into the plate
  │       │  ┌────┐  │       │
  │       │  │stem│  │       │
  │       └──┴────┴──┘       │
  │       ↑ clip  clip ↑     │
  └──────────────────────────┘
          pins hang below
```

### 7b. Mount the ESP32 in the base

1. Place the ESP32 board in the base with the USB-C port aligned to the cutout in the wall.
2. The board should sit flat on the bottom of the case.
3. Secure with a dab of hot glue on two corners, or use double-sided tape.
4. Make sure no solder joints or wires on the bottom of the ESP32 short against anything.

```
  BASE (top view, looking down inside)
  ┌──────────────────────────────┐
  │  ┌────────────────────────┐  │
  │  │                        │  │
  │  │      ESP32 BOARD       │  │
  │  │                        │  │
  │  │   [hot glue corners]   │  │
  │  └────────────────────────┘  │
  │              ↓               │
  │         ┌─────────┐         │
  └─────────┤  USB-C  ├─────────┘
            └─────────┘
              (cutout in wall)
```

### 7c. Route the wires

1. The two wires from the switch (on the top plate) need to reach GPIO 4 and GND on the ESP32 (in the base).
2. Fold the wires neatly so they don't bunch up or get pinched.
3. Make sure no bare wire touches anything conductive on the ESP32 board.

### 7d. Close the case

1. Set the top plate on top of the base. The lip on the underside of the top plate should drop into the base walls.
2. Press down gently — it should friction-fit snugly.
3. If it's too loose, add a tiny bit of hot glue on the inside of the lip.
4. If it's too tight, lightly sand the lip edges.

### 7e. Add the keycap

1. Place the keycap on the switch stem (the cross-shaped post sticking up through the top plate).
2. Press down firmly until it seats — you should feel it snap on.

---

## Step 8: Power On and Pair

1. Plug the USB-C cable into the ESP32 and into your computer (or a power bank).
2. The LED (if installed) should start **slow-blinking** — this means BLE is advertising and waiting for a connection.
3. On your computer, phone, or tablet:
   - Go to **Bluetooth settings**.
   - Look for a device called **"OneKey"**.
   - Click/tap to pair. No PIN is required.
4. Once paired, the LED should turn **solid off** (or stop blinking, depending on your LED mode).
5. **Press the key!**
   - The LED flashes briefly.
   - The configured keystroke is sent to your device.
   - Open a text editor to verify, or try it on a Zoom/Teams call for mute.

---

## Troubleshooting

| Symptom | Check |
|---------|-------|
| LED doesn't light up at all | Verify LED_PIN in config.h matches your wiring. Check LED polarity (long leg = +). Check resistor is connected. |
| "OneKey" doesn't appear in Bluetooth | Open Serial Monitor — is the firmware running? Try resetting the ESP32. Make sure BLE is enabled on your host device. |
| Key press not detected (no "Key DOWN" in Serial Monitor) | Check wiring: use a multimeter to verify continuity between switch pins when pressed. Confirm BUTTON_PIN matches your wiring. |
| Key detected in Serial Monitor but no keystroke on computer | BLE may not be paired. Remove the pairing and re-pair. Try a different host device to isolate the issue. |
| Key fires twice or stutters | Increase DEBOUNCE_MS in config.h (try 80 or 100). |
| Upload fails | Hold the BOOT button on the ESP32 while clicking Upload, release after upload starts. Verify correct board and port in Tools menu. |
| Switch is loose in the top plate | Add hot glue around the switch clips. Or reprint with a slightly smaller cutout (try 13.9mm). |
| Case doesn't fit your board | Re-measure your board and update the dimensions in one_key_case.scad. Add 0.5mm tolerance on each side. |

---

## Finished!

You now have a fully functional single-key Bluetooth keyboard. Here's what you built:

```
  ┌──────────────────────────────┐
  │          ┌────────┐          │
  │          │KEYCAP  │          │
  │          │  ████  │          │
  │          └────────┘          │  ← Top plate with switch
  │══════════════════════════════│
  │  ┌────────────────────────┐  │
  │  │      ESP32 + wires     │  │  ← Base with board
  │  └────────────────────────┘  │
  └───────────┤USB-C├───────────┘
```

To change what the key does, edit `firmware/config.h` and re-flash. No rewiring needed.
