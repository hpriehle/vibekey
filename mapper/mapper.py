#!/usr/bin/env python3
"""
VibeKey Mapper — Configure your 3-button BLE keyboard
======================================================
NOTE: This module is kept for backwards compatibility.
The recommended way to manage your VibeKey is the main CLI:

    ./vibekey map           Interactive key mapping
    ./vibekey setup         Guided first-time setup
    ./vibekey help          All commands

This standalone script still works:
    python3 mapper.py                       # auto-detect serial port
    python3 mapper.py --port /dev/ttyUSB0   # specify port

Requirements:
    pip install pyserial
"""

import sys
import time
import argparse

try:
    import serial
    import serial.tools.list_ports
except ImportError:
    print("Error: pyserial is required.")
    print("Install it with:  pip install pyserial")
    sys.exit(1)

# ── Key name ↔ code lookup table ──────────────────────────────────

KEY_MAP = {
    # Letters
    "a": 97, "b": 98, "c": 99, "d": 100, "e": 101, "f": 102,
    "g": 103, "h": 104, "i": 105, "j": 106, "k": 107, "l": 108,
    "m": 109, "n": 110, "o": 111, "p": 112, "q": 113, "r": 114,
    "s": 115, "t": 116, "u": 117, "v": 118, "w": 119, "x": 120,
    "y": 121, "z": 122,
    # Numbers
    "0": 48, "1": 49, "2": 50, "3": 51, "4": 52,
    "5": 53, "6": 54, "7": 55, "8": 56, "9": 57,
    # Special keys
    "enter": 176, "return": 176,
    "esc": 177, "escape": 177,
    "backspace": 178,
    "tab": 179,
    "space": 32,
    "caps_lock": 193,
    "insert": 218,
    "delete": 214,
    "home": 212,
    "end": 215,
    "page_up": 213,
    "page_down": 216,
    "right": 217, "right_arrow": 217,
    "left": 218, "left_arrow": 218,
    "down": 219, "down_arrow": 219,
    "up": 220, "up_arrow": 220,
    # Function keys
    "f1": 194, "f2": 195, "f3": 196, "f4": 197,
    "f5": 198, "f6": 199, "f7": 200, "f8": 201,
    "f9": 202, "f10": 203, "f11": 204, "f12": 205,
    "f13": 206, "f14": 207, "f15": 208, "f16": 209,
    "f17": 210, "f18": 211, "f19": 212, "f20": 213,
    "f21": 214, "f22": 215, "f23": 216, "f24": 217,
    # Modifiers
    "left_ctrl": 128, "left_shift": 129, "left_alt": 130, "left_gui": 131,
    "right_ctrl": 132, "right_shift": 133, "right_alt": 134, "right_gui": 135,
    # Media keys
    "mute": 226, "media_mute": 226,
    "volume_up": 233, "vol_up": 233, "media_volume_up": 233,
    "volume_down": 234, "vol_down": 234, "media_volume_down": 234,
    "play_pause": 205, "media_play_pause": 205,
    "next_track": 181, "media_next_track": 181,
    "prev_track": 182, "previous_track": 182, "media_previous_track": 182,
    "stop": 183, "media_stop": 183,
}

# Reverse lookup: code → friendly name (pick shortest name)
CODE_TO_NAME = {}
for name, code in KEY_MAP.items():
    if code not in CODE_TO_NAME or len(name) < len(CODE_TO_NAME[code]):
        CODE_TO_NAME[code] = name
# Override with nicer display names for common keys
CODE_TO_NAME.update({
    226: "mute",
    233: "volume_up",
    234: "volume_down",
    205: "play_pause",
    181: "next_track",
    182: "prev_track",
    176: "enter",
    177: "esc",
    178: "backspace",
    179: "tab",
    32:  "space",
})

NUM_BUTTONS = 3

# ── Serial communication ─────────────────────────────────────────

def find_vibekey_port():
    """Auto-detect the ESP32 serial port."""
    ports = serial.tools.list_ports.comports()
    esp_keywords = ["CP210", "CH340", "FTDI", "ESP32", "USB Serial",
                    "USB-SERIAL", "ACM", "ttyUSB", "cu.usb", "cu.SLAB"]
    for port in ports:
        desc = f"{port.description} {port.device}".upper()
        if any(kw.upper() in desc for kw in esp_keywords):
            return port.device
    # Fallback: return first available port
    if ports:
        return ports[0].device
    return None


def send_command(ser, cmd):
    """Send a command and return the response lines."""
    ser.reset_input_buffer()
    ser.write((cmd + "\n").encode())
    time.sleep(0.3)

    lines = []
    deadline = time.time() + 2.0
    while time.time() < deadline:
        if ser.in_waiting:
            line = ser.readline().decode(errors="replace").strip()
            if line:
                lines.append(line)
            deadline = time.time() + 0.5  # extend while data is flowing
        else:
            time.sleep(0.05)
    return lines


def get_mappings(ser):
    """Read current key mappings from the device."""
    lines = send_command(ser, "GET")
    for line in lines:
        if line.startswith("MAP:"):
            codes = [int(x) for x in line[4:].split(",")]
            return codes
    return None


def set_mappings(ser, codes):
    """Write key mappings to the device."""
    cmd = "SET " + ",".join(str(c) for c in codes)
    lines = send_command(ser, cmd)
    for line in lines:
        if line == "OK":
            return True
    return False


# ── Display helpers ───────────────────────────────────────────────

def code_to_display(code):
    """Convert a key code to a human-readable name."""
    if code in CODE_TO_NAME:
        return CODE_TO_NAME[code]
    if 32 <= code <= 126:
        return chr(code)
    return f"code_{code}"


def print_key_categories():
    """Print available keys grouped by category."""
    categories = {
        "Media": ["mute", "volume_up", "volume_down", "play_pause",
                   "next_track", "prev_track", "stop"],
        "Common": ["enter", "esc", "backspace", "tab", "space",
                    "delete", "insert", "home", "end",
                    "page_up", "page_down"],
        "Arrows": ["up", "down", "left", "right"],
        "F-Keys": [f"f{i}" for i in range(1, 25)],
        "Letters": [chr(c) for c in range(97, 123)],
        "Numbers": [str(d) for d in range(10)],
        "Modifiers": ["left_ctrl", "left_shift", "left_alt", "left_gui",
                       "right_ctrl", "right_shift", "right_alt", "right_gui"],
    }

    for cat, keys in categories.items():
        print(f"\n  {cat}:")
        line = "    "
        for k in keys:
            entry = k
            if len(line) + len(entry) + 2 > 72:
                print(line)
                line = "    "
            line += entry + "  "
        if line.strip():
            print(line)


def print_current_mappings(codes):
    """Print the current button→key assignments."""
    print("\n  Current mappings:")
    print("  ─────────────────────────────")
    for i, code in enumerate(codes):
        name = code_to_display(code)
        print(f"  Button {i + 1}  →  {name}  (code {code})")
    print()


# ── Interactive menu ──────────────────────────────────────────────

def interactive_mode(ser):
    """Main interactive loop."""
    print("\n╔══════════════════════════════════╗")
    print("║      VibeKey Key Mapper          ║")
    print("╚══════════════════════════════════╝")

    codes = get_mappings(ser)
    if codes is None:
        print("\nError: Could not read mappings from device.")
        print("Make sure the VibeKey firmware is running.")
        return

    while True:
        print_current_mappings(codes)
        print("  Commands:")
        print("  [1/2/3]  Change button 1, 2, or 3")
        print("  [k]      Show all available key names")
        print("  [q]      Quit")
        print()

        choice = input("  > ").strip().lower()

        if choice == "q":
            print("\nDone. Your mappings are saved on the device.")
            break

        elif choice == "k":
            print_key_categories()

        elif choice in ("1", "2", "3"):
            btn_idx = int(choice) - 1
            current = code_to_display(codes[btn_idx])
            print(f"\n  Button {choice} is currently: {current}")
            print("  Enter a key name (e.g. mute, f13, play_pause, a)")
            print("  or a numeric key code. Type 'k' to see all keys.")
            print()

            while True:
                key_input = input(f"  Button {choice} → ").strip().lower()

                if key_input == "k":
                    print_key_categories()
                    continue

                if not key_input:
                    print("  (cancelled)")
                    break

                # Try as a key name
                if key_input in KEY_MAP:
                    new_code = KEY_MAP[key_input]
                # Try as a single character
                elif len(key_input) == 1 and 32 <= ord(key_input) <= 126:
                    new_code = ord(key_input)
                # Try as a numeric code
                elif key_input.isdigit():
                    new_code = int(key_input)
                    if new_code > 255:
                        print("  Error: code must be 0-255")
                        continue
                else:
                    print(f"  Unknown key: '{key_input}'. Type 'k' for the list.")
                    continue

                codes[btn_idx] = new_code
                name = code_to_display(new_code)

                if set_mappings(ser, codes):
                    print(f"  ✓ Button {choice} set to: {name}")
                else:
                    print("  Error: failed to save to device")
                break

        else:
            print("  Unknown command. Type 1, 2, 3, k, or q.")


# ── CLI entry point ───────────────────────────────────────────────

def list_ports():
    """Print all available serial ports."""
    ports = serial.tools.list_ports.comports()
    if not ports:
        print("No serial ports found.")
        return
    print("Available serial ports:")
    for p in ports:
        print(f"  {p.device}  —  {p.description}")


def main():
    parser = argparse.ArgumentParser(
        description="VibeKey Mapper — configure your 3-button BLE keyboard"
    )
    parser.add_argument(
        "--port", "-p",
        help="Serial port (e.g. /dev/ttyUSB0, COM3). Auto-detected if omitted."
    )
    parser.add_argument(
        "--list-ports", "-l",
        action="store_true",
        help="List available serial ports and exit."
    )
    parser.add_argument(
        "--baud", "-b",
        type=int, default=115200,
        help="Baud rate (default: 115200)"
    )
    parser.add_argument(
        "--set",
        metavar="KEY",
        nargs=3,
        help="Non-interactive: set all 3 keys at once. e.g. --set mute volume_down volume_up"
    )
    parser.add_argument(
        "--get",
        action="store_true",
        help="Non-interactive: print current mappings and exit."
    )
    args = parser.parse_args()

    if args.list_ports:
        list_ports()
        return

    port = args.port or find_vibekey_port()
    if port is None:
        print("Error: No serial port found.")
        print("Connect your ESP32 via USB and try again, or specify --port.")
        sys.exit(1)

    print(f"Connecting to {port} at {args.baud} baud...")

    try:
        ser = serial.Serial(port, args.baud, timeout=1)
    except serial.SerialException as e:
        print(f"Error opening {port}: {e}")
        sys.exit(1)

    # Wait for device to be ready
    time.sleep(1.5)
    ser.reset_input_buffer()

    # Verify it's a VibeKey device
    lines = send_command(ser, "PING")
    if not any("PONG:vibekey" in l for l in lines):
        print("Warning: Device did not respond to PING. It may not be running VibeKey firmware.")
        print("Continuing anyway...\n")

    # Non-interactive: --get
    if args.get:
        codes = get_mappings(ser)
        if codes:
            for i, code in enumerate(codes):
                print(f"Button {i+1}: {code_to_display(code)} ({code})")
        else:
            print("Error: could not read mappings")
        ser.close()
        return

    # Non-interactive: --set
    if args.set:
        codes = []
        for key_input in args.set:
            key_input = key_input.lower()
            if key_input in KEY_MAP:
                codes.append(KEY_MAP[key_input])
            elif key_input.isdigit():
                codes.append(int(key_input))
            else:
                print(f"Error: unknown key '{key_input}'")
                ser.close()
                sys.exit(1)
        if set_mappings(ser, codes):
            print("Mappings saved:")
            for i, code in enumerate(codes):
                print(f"  Button {i+1}: {code_to_display(code)}")
        else:
            print("Error: failed to save mappings")
        ser.close()
        return

    # Interactive mode
    try:
        interactive_mode(ser)
    except KeyboardInterrupt:
        print("\n\nInterrupted. Bye!")
    finally:
        ser.close()


if __name__ == "__main__":
    main()
