# VibeKey Shopping List

Everything you need to build a wireless 3-button BLE macro keyboard.

## Electronics

| # | Part | Qty | ~Price | Link |
|---|------|-----|--------|------|
| 1 | **ESP32-S3 DevKitC-1** (N8R8 or N16R8) | 1 | ~$10-15 | [Amazon](https://www.amazon.com/Espressif-ESP32-S3-DevKitC-1-N8R8-Development-Board/dp/B09MHP42LY) |
| 2 | **Cherry MX Switches** (Red=linear, Blue=clicky, Brown=tactile) | 3+ | ~$8-12 (10-pack) | [Amazon](https://www.amazon.com/10-Pack-Cherry-switch-Mechanical-keyboard/dp/B06XFSWPY3) |
| 3 | **Blank MX Keycaps** (1U, PBT) | 3+ | ~$6-8 (10-pack) | [Amazon](https://www.amazon.com/PATIKIL-Universal-Replacement-Accessories-Mechanical/dp/B0B5KZQ9X9) |
| 4 | **3.7V 500mAh LiPo Battery** (JST PH2.0 connector) | 1 | ~$8 | [Amazon](https://www.amazon.com/battery-Rechargeable-Lithium-Polymer-Connector/dp/B07BTKPSNP) |
| 5 | **TP4056 USB-C Charging Board** (with protection circuit) | 1+ | ~$6 (3-pack) | [Amazon](https://www.amazon.com/HiLetgo-Lithium-Charging-Protection-Functions/dp/B07PKND8KG) |
| 6 | **SPDT Slide Switch** (mini, through-hole) | 1+ | ~$6 (20-pack) | [Amazon](https://www.amazon.com/CHANZON-RA12-SLIDE-SWITCH/dp/B09XQQSWC6) |
| 7 | **100K Ohm Resistors** (1/4W, through-hole) | 2+ | ~$6 (pack) | [Amazon](https://www.amazon.com/100k-ohm-resistor/s?k=100k+ohm+resistor) |

## Wiring & Soldering

| # | Part | Qty | ~Price | Notes |
|---|------|-----|--------|-------|
| 8 | **Hook-up wire** (22-26 AWG, solid core) | 1 spool | ~$7 | For connecting switches to ESP32 |
| 9 | **Solder + soldering iron** | 1 | varies | If you don't already have one |
| 10 | **USB-C cable** | 1 | ~$5 | For flashing firmware & charging |

## 3D Printed Case

| # | Part | Notes |
|---|------|-------|
| 11 | **Base** | Print from `case/one_key_case.scad` (export base as STL) |
| 12 | **Top plate** | Print from `case/one_key_case.scad` (export top plate as STL) |

Print settings: PLA or PETG, 0.2mm layer height, 20% infill, no supports needed.

## Optional

| # | Part | Why |
|---|------|-----|
| 13 | M2 self-tapping screws (6mm) x4 | If using screw mounts instead of friction fit |
| 14 | 3mm LED + resistor (220Ω) | Status LED for BLE connection |
| 15 | Heat shrink tubing | Clean up solder joints |
| 16 | JST PH2.0 pigtail cable | Easier battery wiring if your battery doesn't come with leads |

## Estimated Total: ~$45-60

(Most parts come in multi-packs, so you'll have spares for future builds.)

## Before You Print

Measure your actual parts and update the dimensions at the top of `case/one_key_case.scad`:
- `board_width`, `board_length`, `board_height` — your ESP32 board
- `batt_width`, `batt_length`, `batt_height` — your LiPo cell
- `tp_width`, `tp_length` — your TP4056 board
- `sw_width`, `sw_height` — your slide switch
