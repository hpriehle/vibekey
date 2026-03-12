/*
 * OneKey Keyboard Case
 * ====================
 * A simple 3D-printable enclosure for a single MX key switch
 * and an ESP32 dev board.
 *
 * Usage:
 *   1. Open this file in OpenSCAD (free: openscad.org)
 *   2. Adjust the dimensions below to match your specific ESP32 board
 *   3. Render one part at a time:
 *      - Comment out top_plate() to export the base
 *      - Comment out base() to export the top plate
 *   4. Export as STL -> slice -> print
 *
 * Print Settings:
 *   Layer height: 0.2mm
 *   Infill: 20%
 *   Supports: None needed
 *   Material: PLA or PETG
 */

// -- DIMENSIONS (mm) -- Adjust these to fit your board ------

// ESP32 board dimensions (measure your specific board!)
board_width  = 28;    // Width of the ESP32 PCB
board_length = 52;    // Length of the ESP32 PCB
board_height = 12;    // Height clearance for components on the bottom

// Case properties
wall = 2;             // Wall thickness
bottom = 2;           // Bottom plate thickness
tolerance = 0.5;      // Extra space around the board for fit

// MX switch cutout (standard -- don't change)
switch_cutout = 14;   // 14mm x 14mm is MX standard
switch_plate_thickness = 1.5;  // Plate thickness for switch clip

// USB port cutout
usb_width  = 12;      // Width of USB-C connector + clearance
usb_height = 7;       // Height of USB-C connector + clearance

// -- CALCULATED VALUES --------------------------------------

inner_w = board_width + tolerance * 2;
inner_l = board_length + tolerance * 2;
outer_w = inner_w + wall * 2;
outer_l = inner_l + wall * 2;
case_height = bottom + board_height;

// -- RENDER CONTROL -----------------------------------------
// Comment/uncomment to export one part at a time

base();
translate([outer_w + 10, 0, 0]) top_plate();

// -- BASE MODULE --------------------------------------------
module base() {
  difference() {
    // Outer shell
    cube([outer_w, outer_l, case_height]);

    // Inner cavity
    translate([wall, wall, bottom])
      cube([inner_w, inner_l, case_height]);

    // USB-C port cutout (on the short side, centered)
    translate([(outer_w - usb_width) / 2, -0.1, bottom + board_height - usb_height])
      cube([usb_width, wall + 0.2, usb_height + 1]);
  }
}

// -- TOP PLATE MODULE ---------------------------------------
module top_plate() {
  plate_w = outer_w;
  plate_l = outer_l;
  plate_h = switch_plate_thickness;

  // Lip dimensions (sits inside the base walls)
  lip_depth = 2;
  lip_inset = 0.3;  // Slight inset for friction fit

  difference() {
    union() {
      // Main plate
      cube([plate_w, plate_l, plate_h]);

      // Inner lip that drops into the base
      translate([wall + lip_inset, wall + lip_inset, -lip_depth])
        cube([inner_w - lip_inset * 2, inner_l - lip_inset * 2, lip_depth]);
    }

    // MX switch cutout (centered on the plate)
    translate([(plate_w - switch_cutout) / 2, (plate_l - switch_cutout) / 2, -0.1])
      cube([switch_cutout, switch_cutout, plate_h + lip_depth + 0.2]);
  }
}
