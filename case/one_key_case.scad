/*
 * VibeKey 3-Button Keyboard Case
 * ================================
 * A 3D-printable enclosure for three MX key switches
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

// -- CONFIGURATION (mm) -- Adjust these to fit your board --------

// Number of keys
num_keys = 3;

// ESP32 board dimensions (measure your specific board!)
board_width  = 28;    // Width of the ESP32 PCB
board_length = 52;    // Length of the ESP32 PCB
board_height = 12;    // Height clearance for components on the bottom

// Case properties
wall = 2;             // Wall thickness
bottom = 2;           // Bottom plate thickness
tolerance = 0.5;      // Extra space around the board for fit
corner_r = 2;         // Corner rounding radius

// MX switch cutout (standard -- don't change)
switch_cutout = 14;   // 14mm x 14mm is MX standard
switch_plate_thickness = 1.5;  // Plate thickness for switch clip
switch_spacing = 2;   // Gap between switch cutouts

// USB port cutout
usb_width  = 12;      // Width of USB-C connector + clearance
usb_height = 7;       // Height of USB-C connector + clearance

// LED hole (set to 0 to disable)
led_hole_dia = 3.2;   // Diameter for 3mm LED press-fit

// Screw mount (set to 0 to use friction fit instead)
screw_dia = 2.2;      // M2 self-tapping screw hole
screw_boss_dia = 5;   // Outer diameter of screw boss
screw_depth = 6;      // Depth of screw hole

// -- CALCULATED VALUES ------------------------------------------

// Top plate width: enough for all switches side by side with spacing
keys_width = num_keys * switch_cutout + (num_keys - 1) * switch_spacing;
// Inner width is the wider of the board or the keys row, plus tolerance
inner_w = max(board_width, keys_width) + tolerance * 2;
inner_l = board_length + tolerance * 2;
outer_w = inner_w + wall * 2;
outer_l = inner_l + wall * 2;
case_height = bottom + board_height;

// -- RENDER CONTROL ---------------------------------------------
// Comment/uncomment to export one part at a time

base();
translate([outer_w + 10, 0, 0]) top_plate();

// -- MODULES ----------------------------------------------------

module rounded_box(w, l, h, r) {
    // A box with rounded vertical edges
    hull() {
        translate([r, r, 0]) cylinder(h=h, r=r, $fn=32);
        translate([w-r, r, 0]) cylinder(h=h, r=r, $fn=32);
        translate([r, l-r, 0]) cylinder(h=h, r=r, $fn=32);
        translate([w-r, l-r, 0]) cylinder(h=h, r=r, $fn=32);
    }
}

module base() {
    difference() {
        // Outer shell with rounded corners
        rounded_box(outer_w, outer_l, case_height, corner_r);

        // Inner cavity
        translate([wall, wall, bottom])
            rounded_box(inner_w, inner_l, case_height, max(0.1, corner_r - wall));

        // USB-C port cutout (on the short side, centered)
        translate([(outer_w - usb_width) / 2, -0.1, bottom + board_height - usb_height])
            cube([usb_width, wall + 0.2, usb_height + 1]);

        // LED hole on the front face (opposite USB side)
        if (led_hole_dia > 0) {
            translate([outer_w / 2, outer_l + 0.1, bottom + board_height / 2])
                rotate([90, 0, 0])
                    cylinder(d=led_hole_dia, h=wall + 0.2, $fn=24);
        }
    }

    // Screw bosses in corners (optional)
    if (screw_dia > 0) {
        for (pos = [
            [wall + screw_boss_dia/2, wall + screw_boss_dia/2],
            [outer_w - wall - screw_boss_dia/2, wall + screw_boss_dia/2],
            [wall + screw_boss_dia/2, outer_l - wall - screw_boss_dia/2],
            [outer_w - wall - screw_boss_dia/2, outer_l - wall - screw_boss_dia/2],
        ]) {
            translate([pos[0], pos[1], bottom])
                difference() {
                    cylinder(d=screw_boss_dia, h=screw_depth, $fn=24);
                    translate([0, 0, -0.1])
                        cylinder(d=screw_dia, h=screw_depth + 0.2, $fn=16);
                }
        }
    }
}

module top_plate() {
    plate_w = outer_w;
    plate_l = outer_l;
    plate_h = switch_plate_thickness;

    // Lip dimensions (sits inside the base walls)
    lip_depth = 2;
    lip_inset = 0.3;  // Slight inset for friction fit

    difference() {
        union() {
            // Main plate with rounded corners
            rounded_box(plate_w, plate_l, plate_h, corner_r);

            // Inner lip that drops into the base
            translate([wall + lip_inset, wall + lip_inset, -lip_depth])
                rounded_box(
                    inner_w - lip_inset * 2,
                    inner_l - lip_inset * 2,
                    lip_depth,
                    max(0.1, corner_r - wall - lip_inset)
                );
        }

        // MX switch cutouts — centered as a row
        row_start_x = (plate_w - keys_width) / 2;
        center_y = plate_l / 2 - switch_cutout / 2;

        for (i = [0 : num_keys - 1]) {
            x = row_start_x + i * (switch_cutout + switch_spacing);
            translate([x, center_y, -lip_depth - 0.1])
                cube([switch_cutout, switch_cutout, plate_h + lip_depth + 0.2]);
        }

        // Screw holes (matching base bosses)
        if (screw_dia > 0) {
            for (pos = [
                [wall + screw_boss_dia/2, wall + screw_boss_dia/2],
                [plate_w - wall - screw_boss_dia/2, wall + screw_boss_dia/2],
                [wall + screw_boss_dia/2, plate_l - wall - screw_boss_dia/2],
                [plate_w - wall - screw_boss_dia/2, plate_l - wall - screw_boss_dia/2],
            ]) {
                translate([pos[0], pos[1], -lip_depth - 0.1])
                    cylinder(d=screw_dia + 0.3, h=plate_h + lip_depth + 0.2, $fn=16);
            }
        }
    }
}
