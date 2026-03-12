/*
 * VibeKey 3-Button Keyboard Case (Battery Edition)
 * ==================================================
 * A 3D-printable enclosure for three MX key switches,
 * an ESP32 dev board, a LiPo battery, and a TP4056
 * USB-C charging board.
 *
 * Layout (top view, Y axis is length):
 *   +-----------------+
 *   |  [TP4056] [sw]  |  <-- USB-C end (Y=0)
 *   |    [key 1]      |
 *   |    [key 2]      |
 *   |    [key 3]      |
 *   |  [  battery  ]  |  <-- far end
 *   +-----------------+
 *
 * Usage:
 *   1. Open this file in OpenSCAD (free: openscad.org)
 *   2. Adjust the dimensions below to match your specific parts
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

// -- BATTERY & CHARGING CONFIGURATION --
// LiPo battery dimensions (default: 500mAh pouch cell)
batt_width  = 40;     // Battery width (X)
batt_length = 30;     // Battery length (Y)
batt_height = 6;      // Battery thickness (Z)
batt_tolerance = 0.5; // Extra clearance around battery

// TP4056 USB-C charging board dimensions
tp_width  = 17;       // Board width (X) -- mounted with USB facing case wall
tp_length = 26;       // Board length (Y)
tp_height = 4;        // Board + component height (Z)
tp_tolerance = 0.3;   // Fit clearance

// Power switch cutout (slide switch on side wall)
sw_width  = 13;       // Switch body length along wall (Y)
sw_height = 8;        // Switch body height (Z)

// LED hole (set to 0 to disable)
led_hole_dia = 3.2;   // Diameter for 3mm LED press-fit

// Screw mount (set to 0 to use friction fit instead)
screw_dia = 2.2;      // M2 self-tapping screw hole
screw_boss_dia = 5;   // Outer diameter of screw boss
screw_depth = 6;      // Depth of screw hole

// -- CALCULATED VALUES ------------------------------------------

// Top plate width: enough for all switches side by side with spacing
keys_length = num_keys * switch_cutout + (num_keys - 1) * switch_spacing;

// Inner width must fit the widest component (board or battery — keys now run along Y)
inner_w = max(board_width, batt_width + batt_tolerance * 2) + tolerance * 2;

// Inner length: ESP32 + divider wall + battery compartment
divider_wall = 1.5;   // Thin wall separating ESP32 from battery
batt_compartment_l = batt_length + batt_tolerance * 2;
inner_l = max(board_length, keys_length) + tolerance * 2 + divider_wall + batt_compartment_l;

outer_w = inner_w + wall * 2;
outer_l = inner_l + wall * 2;

// Case height: must clear the tallest stack (board components or battery)
case_height = bottom + max(board_height, batt_height + batt_tolerance + 1);

// Y position where the battery compartment starts (inside the cavity)
batt_comp_y_start = board_length + tolerance * 2 + divider_wall;

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

        // -------------------------------------------------------
        // Main inner cavity (full length: ESP32 + battery zones)
        // -------------------------------------------------------
        translate([wall, wall, bottom])
            rounded_box(inner_w, inner_l, case_height, max(0.1, corner_r - wall));

        // -------------------------------------------------------
        // USB-C port cutout (Y=0 wall, centered in X)
        // Used by the TP4056 charging board's USB-C connector.
        // -------------------------------------------------------
        translate([(outer_w - usb_width) / 2, -0.1, bottom + board_height - usb_height])
            cube([usb_width, wall + 0.2, usb_height + 1]);

        // -------------------------------------------------------
        // Power switch cutout (slide switch on right side wall)
        // Positioned in the ESP32 zone, centered vertically on
        // the wall so wires can reach the TP4056 output pads.
        // -------------------------------------------------------
        translate([
            outer_w - wall - 0.1,                         // right wall
            wall + (board_length + tolerance * 2 - sw_width) / 2,  // centered in ESP32 zone
            bottom + (case_height - bottom - sw_height) / 2        // centered vertically
        ])
            cube([wall + 0.2, sw_width, sw_height]);

        // -------------------------------------------------------
        // LED hole on the far face (opposite USB side)
        // -------------------------------------------------------
        if (led_hole_dia > 0) {
            translate([outer_w / 2, outer_l + 0.1, bottom + board_height / 2])
                rotate([90, 0, 0])
                    cylinder(d=led_hole_dia, h=wall + 0.2, $fn=24);
        }

        // -------------------------------------------------------
        // Battery cavity — a shallow pocket in the floor of the
        // battery compartment.  The pocket is batt_height deep so
        // the battery sits flush or slightly below the divider,
        // keeping the centre of gravity low and leaving room for
        // wiring above.
        // -------------------------------------------------------
        translate([
            wall + (inner_w - batt_width - batt_tolerance * 2) / 2,  // centered in X
            wall + batt_comp_y_start + batt_tolerance,                // inside battery zone
            bottom                                                    // floor level
        ])
            cube([
                batt_width + batt_tolerance * 2,
                batt_length + batt_tolerance * 2,
                batt_height + batt_tolerance
            ]);
    }

    // -------------------------------------------------------
    // Divider wall between ESP32 and battery compartments.
    // A thin rib across the width with a wire channel notch
    // so power leads can pass through.
    // -------------------------------------------------------
    wire_channel_w = 6;   // width of notch for wires
    wire_channel_h = 4;   // height of notch
    translate([wall, wall + board_length + tolerance * 2, bottom])
        difference() {
            cube([inner_w, divider_wall, case_height - bottom]);
            // Wire pass-through notch (centered at bottom)
            translate([(inner_w - wire_channel_w) / 2, -0.1, 0])
                cube([wire_channel_w, divider_wall + 0.2, wire_channel_h]);
        }

    // -------------------------------------------------------
    // TP4056 board mount — two guide rails inside the ESP32
    // zone near the USB-C wall.  The charging board slides in
    // from above and sits against the front wall so its USB-C
    // port aligns with the case cutout.
    // -------------------------------------------------------
    tp_rail_h = tp_height + 1;          // rail height
    tp_rail_w = 1.2;                    // rail wall thickness
    tp_slot_x = (outer_w - tp_width - tp_tolerance * 2) / 2;  // centered

    // Left guide rail
    translate([tp_slot_x - tp_rail_w, wall, bottom])
        cube([tp_rail_w, tp_length + tp_tolerance, tp_rail_h]);
    // Right guide rail
    translate([tp_slot_x + tp_width + tp_tolerance * 2, wall, bottom])
        cube([tp_rail_w, tp_length + tp_tolerance, tp_rail_h]);

    // -------------------------------------------------------
    // Screw bosses in corners (optional)
    // -------------------------------------------------------
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

        // MX switch cutouts — centered as a column along Y (long axis)
        center_x = (plate_w - switch_cutout) / 2;
        row_start_y = (plate_l - keys_length) / 2;

        for (i = [0 : num_keys - 1]) {
            y = row_start_y + i * (switch_cutout + switch_spacing);
            translate([center_x, y, -lip_depth - 0.1])
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
