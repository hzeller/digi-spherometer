// -*- mode: scad; c-basic-offset: 2; indent-tabs-mode: nil; -*-
// Mounting frame holding the indicator and display
// CAVE: Super-hacky right now while exploring different shape ideas.
// Now that it is more clean: need some clean-up.
//
// Rough nomenclature:
// Something called 'punch' is a negative space.
//
// Parts often come in two modules: foo() and foo_punch(). foo() defines the
// outer size, while foo_punch() the negative space. They are separate as often
// negative spaces are applied after part is built up (e.g. you want to have
// screw holes punch through all assembled parts).
//
// Some parts also come with a foo_separator(). This defines some cut through
// a part (somtimes with a complicated shape), which can separate a larger
// part using a difference() into one half and intersection() to the other half.
// Often fit_tolerance plays a role her as well.


// The following variables are set in the makefile.
print_quality=false;      // print quality: high-res, but slow to render.
version_id="git-hash";    // Identify version for easier re-print
version_date="git-date";  // .. and date of that version.

// Either fast or pretty.
$fs=print_quality ? 0.15 : 1;  // Half the size the printer can do.
$fa=print_quality ? 1 : 6;

e=0.01;             // Epsilon to reliably punch holes
fit_tolerance=0.3;  // Tolerance of parts in contact.

m3_dia=3.4;         // Let it a little loose to not overconstrain things.
m3_head_dia=6;
m3_head_len=3;
m3_nut_dia=5.4 / cos(30) + 2*fit_tolerance;  // div by cos(30) for circumcircle
m3_nut_thick=2.8;

// Sperometer legs.
leg_radius=50;
leg_plate_thick=12.7;    // Thickness of the acrylic used.
leg_ball_dia=12.7;
leg_ball_hole_dia=8;
leg_plate_rim=7;         // Extra acrylic beyond the legs.
leg_plate_radius=127/2; //leg_radius + leg_ball_hole_dia/2 + leg_plate_rim;

dial_dia=57.5;
dial_cable_thick=1.5;   // Thickness of the data cables.
dial_cable_wide = 6 * dial_cable_thick;  // 4-5 wires + little slack.
dial_wall=dial_cable_thick+1;
dial_thick=25.2;
dial_stem_pos = 21.4-4;  // Position of stem from the frontface
dial_cable_pos=12;   // Position of the cable channel from the front

// Stem of the indicator, the bushing the measuring-rod is moving in.
// Parameters from the autolet indicator.
stem_dia=8;                       // Stem of the meter
stem_bushing_len=21.5;            // How long is the stem bushing
stem_high=stem_bushing_len - leg_plate_thick - dial_wall;
// Mount distance is at least one M3 diameter (2*half an M3), plus some distance
// right and left.
stem_mount_screw_distance=stem_dia + 3 + 2*1.5;

// Battery sizes
aa_dia=14.5 + 2*fit_tolerance;
aa_len=50.5 + 7;  // for contacts
aa_wall=3;
aa_dist = 8;      // Distance between batteries

// Battery box wiggle printing on the back is a challenge. Maybe this needs to
// be split in the wiggle part and flat battery box back, that are then glued
// together.
battery_box_with_wiggle=true;   // Easier to print if false :)
battery_box_rim_deep=1.5;       // Alignment rim all around battery box.
battery_center_tap = false;     // Add channel for 1.5V center tap

base_dia=dial_stem_pos*2;

display_wall_thick=1.5;
display_top_wall=0.8;    // Front face a little thinner to have display close.
display_wide=52-0;         // Mostly determined by electronics size and screws.
display_high=36;
display_box_thick=7.5;   // Large enough to house electronics.
display_transition=7;    // Transition blend between dial and display box.
display_front_radius_adjust=-0.5;  // Don't go all the way to the edge of plate
display_button_from_top=10.11;     // Position of the button

// Mounting holes, holding down the back part, the front part and the
// display part.

// The Squeeze-block is the block squeezing in the stem.
squeeze_block_mount = false;  // Should we have an extra mount hole for sq-block
squeeze_block_wide = 2 * stem_mount_screw_distance;

cable_channel_from_bottom = 2.5;    // Cable slot access from the bottom.
cable_channel_distance=squeeze_block_wide / 2;
cable_channel_front_wide=3;

front_width=squeeze_block_wide + 2*cable_channel_front_wide + 2*m3_nut_dia + 4;
// TODO: Naming of these is confusing.
bottom_mount_front_offset=10;  // Bottom screws. Offset from center to back.
bottom_mount_center_offset=bottom_mount_front_offset + 0;
bottom_mount_front_distance=front_width - 8;  // right/left distance.

// Center screw in display-front.
bottom_mount_front_display_center_offset=leg_plate_radius - 8;

bottom_mount_back_offset=7;  // Bottom screws. Offset from center to back.
bottom_mount_back_distance=display_wide - 13;  // right/left distance.

function max(a, b) = a > b ? a : b;

// m3_screw space occupied by a M3 screw with optional nut and
// nut-access channel. Screw is centered around the Z-axis, with the
// screw in the positive and screw-head in the negative range.
// nut_at: start of where a m3 nut shoud be placed. -1 for off.
// nut_channel: make a channel of given length to slide a nut in.
//              nut channel extends in negative Y direction. Rotate as needed.
module m3_screw(len=60, nut_at=-1, nut_channel=-1, nut_thick=m3_nut_thick) {
  cylinder(r=m3_dia/2, h=len);
  translate([0, 0, -20+e]) cylinder(r=m3_head_dia/2, h=20);
  if (nut_at >= 0) {
    translate([0, 0, nut_at]) {
      rotate([0, 0, 30]) cylinder(r=m3_nut_dia/2, h=nut_thick, $fn=6);
      nut_wide=m3_nut_dia * cos(30);
      if (nut_channel > 0) {
        nut_channel_len = max(nut_channel, m3_nut_dia/2);
        translate([-nut_wide/2, -nut_channel_len, 0]) cube([nut_wide, nut_channel_len, nut_thick]);
      }
    }
  }
}

// A block that looks like a cut through a cosine curve. Size parameter
// similar to cube(), but x/y direction is symmetric to center, so the
// actual width/height is twice. (TODO: make more like cube)
module cos_block(a=[10,10,10]) {
  p = [ for (x = [-180:1:180]) [ x/180, cos(x)/2] ];
  linear_extrude(height=a[2]) scale(a) polygon(p);
}

// Make a 2D shape of length in X direction of "len", height in y-direction
// of "Y" that has "wiggle_count" bumps.
module sine_wiggle(wiggle_count=3, len=20, height=2, resolution=0.01) {
  points = [ for (w = [0 : resolution : 1.0]) [ w * len, height/2 + height/2 * sin(w*wiggle_count*360-90)] ];
  points1=concat([[0, -e]], points, [[len, -e]]);
  translate([-len/2, 0, 0]) polygon(points1);
}

// A corner for a champfered box, represented as tiny objects for which it
// is simple to wrap a hul() around.
// this is for a front-left corner. Origin is at the bottom end of the corner,
// the x and y direction size allow for different champfer angles in x and y
// direction.
// To use in different corners, it is probably easiest to mirror it (
// using scale around the origin).
module champfer_point_cloud(height=display_box_thick, side_x=3, side_y=8) {
  a=e;
  f=0;  // front champfer
  cube([a, a, a]);
  translate([0, side_y, height]) cube([a, a, a]);
  translate([side_x, f, height]) cube([a, a, a]);
  translate([side_x, 0, height-f]) cube([a, a, a]);
}

// Stem of indicator.
module stem_punch() {
  // A little thinner in y direction to have enough 'squeeze' action.
  scale([1, 0.98, 1]) translate([0, 0, -25]) cylinder(r=stem_dia/2, h=50);
}

// Base where everything is sitting on the bottom.
module base(with_front_flat=true) {
  scale([1, 1, 1]) cylinder(r=base_dia/2, h=0.8);
  translate([-front_width/2, -base_dia/2, 0]) cube([front_width, base_dia/2, 2]);
  if (with_front_flat) {
    translate([-front_width/2, -base_dia/2, 0]) cube([front_width, 1, stem_high]);
  }
}

// Some text helping to identify from which git-hash this has been printed for
// easier re-print.
module version_punch(thick=1) {
  fs=5;
  color("red") linear_extrude(height=thick) {
    text("© H. Zeller", halign="center", size=fs);
    translate([0, -1*(fs+1)]) text(version_date, halign="center", size=fs);
    translate([0, -2*(fs+1)]) text(version_id, halign="center", size=fs);
  }
}

// Outer case to hold the dial.
module dial_holder() {
  wall_r =dial_dia/2 + dial_wall;
  rotate([90, 0, 0]) translate([0, wall_r+stem_high, -dial_thick+dial_stem_pos]) cylinder(r=wall_r, h=dial_thick);
}

module dial_holder_cable_punch() {
  connector_from_top=13;
  wall_r = dial_dia/2 + dial_wall;
  connector_angle_from_top=90
    + 360 * (-dial_cable_wide - connector_from_top) / (dial_dia * PI);
  connector_angle_width=360 * dial_cable_wide / (dial_dia * PI);
  hull() {
    bottom_wide=dial_cable_wide;
    back_channel_depth=1.2*dial_cable_thick;
    // Front channel access
    translate([cable_channel_distance, -dial_stem_pos-e, cable_channel_from_bottom]) cube([cable_channel_front_wide, 1, stem_high+10]);

    // Bottom back channel
    translate([cable_channel_distance-bottom_wide/2, dial_thick-dial_stem_pos, dial_wall+7]) cube([bottom_wide, back_channel_depth, dial_cable_thick]);

    // top channel between connector and back.
    rotate([90, 0, 0])
      translate([0, wall_r+stem_high, -dial_thick+dial_stem_pos])
      translate([0, 0, -back_channel_depth]) rotate([0, 0, connector_angle_from_top]) rotate_extrude(angle=connector_angle_width, convexity=2) translate([dial_dia/2, 0, 0]) square([dial_cable_thick, dial_thick+back_channel_depth]);
  }
}

// Negative space to hold the dial and access holes for stem and cables.
module dial_holder_punch(cable_slot=true) {
  extra=40;
  wall_r =dial_dia/2 + dial_wall;
  connector_from_top=13;

  rotate([90, 0, 0])
    translate([0, wall_r+stem_high, -dial_thick+dial_stem_pos]) {

    // The dial. With extra punch to the front.
    cylinder(r=dial_dia/2, h=dial_thick+extra);
    translate([-7, 12, -0.5]) version_punch(0.5);

    // Leave the top free. We punch out a nice cosine-shaped wobble
    top_punch=0.9*connector_from_top;
    translate([0,0,e]) rotate([-90, 0, 0]) translate([0, -dial_thick/2, 0]) cos_block([2*top_punch, dial_thick, dial_dia]);
  }
  stem_punch();

  // More punch for cable management: lead it out on the back into the slot
  if (cable_slot) color("red") dial_holder_cable_punch();
}

// Punching space for direction battery pictogram and +/- designations.
module aa_pictogram_punch(h=3) {
  translate([0, 0, -h+0.5]) color("red") {
    translate([0, -aa_len*0.25-8, 0]) linear_extrude(height=h) text("–", halign="center");
    translate([0, aa_len*0.25, 0]) linear_extrude(height=h) text("+", halign="center");
    translate([0, 0, h/2]) difference() {
      union() {
        cube([6, 20, h], center=true);
        translate([0, 10, 0]) cube([3, 4, h], center=true);
      }
      cube([6-2, 20-2, h+2*e], center=true);
    }
  }
}

// Punching space for a AA-Battery including direction pictogram. Provides
// a recangular access path to insert the battery.
// Battery alongside Z axis, centered around X and Y axis.
// Pictograms on negative Y side, battery insert on negative Y-side
module aa_bat_punch(block_recess=3) {
  translate([0, 0, -aa_len/2]) {
    cylinder(r=aa_dia/2, h=aa_len);
    translate([-aa_dia/2, 0, 0]) cube([aa_dia, aa_dia/2-block_recess, aa_len]);
    translate([0, -aa_dia/2, aa_len/2]) rotate([90, 0, 0]) aa_pictogram_punch();
  }
}

// Box to hold batteries.
module battery_box(with_wiggle=false) {
  height=aa_len + 2*aa_wall;
  width=2*aa_dia + aa_dist + 2*aa_wall;
  thick=aa_dia+2*aa_wall;
  difference() {
    union() {
      cc=2;  // Champfer on top.
      rotate([90, 0, 0]) translate([-width/2, 0, -thick])
        linear_extrude(height=thick)
        polygon([[0, 0], [0, height-cc], [cc, height],
                 [width-cc, height], [width, height-cc],
                 [width, 0]]);
      if (with_wiggle) {
        translate([-width/2, thick, height/2]) rotate([0, 90, 0])
          linear_extrude(height=width, convexity=10) sine_wiggle(len=height);
      }
    }
    battery_box_punch();
  }
}

// Punching space for two batteries, battery lid screw and 1.5V tap cable space.
module battery_box_punch() {
  height=aa_len + 2*aa_wall;
  width=2*aa_dia + aa_dist + 2*aa_wall;
  thick=aa_dia+2*aa_wall;

  translate([0, thick/2, 0]) {
    translate([-(aa_dia+aa_dist)/2, 0, aa_wall+aa_len/2]) aa_bat_punch();
    translate([+(aa_dia+aa_dist)/2, 0, aa_wall+aa_len/2]) rotate([0, 180, 0]) aa_bat_punch();

    // battery lugs. Meant to be 'worked' by solder heating
    lug_thick=0.5;
    lug_wide=6;
    translate([0, 0, aa_wall+lug_thick/2]) cube([aa_dist+2, lug_wide, lug_thick], center=true);
    // Don't bridge all the way. All we want to do is to add a cavity that then
    // has to be opened up again. But this way we get a multi-wall surrounded
    // cavity while filament bridging is not impaired too much.
    translate([0, 0, aa_len+aa_wall-lug_thick/2])
      cube([aa_dist-2, lug_wide, lug_thick], center=true);
    translate([-(aa_dist/2+aa_dia), 0, aa_wall+lug_thick/2]) cube([5, lug_wide, lug_thick], center=true);

    // Cable for 1.5V tap.
    if (battery_center_tap) {
      translate([-(aa_dist+aa_dia)/2, 0, 0]) rotate([0, 0, -45]) translate([aa_dia/2, 0, aa_wall]) cylinder(r=dial_cable_thick/2, h=aa_len);
    }
    empty_space_fraction=0.0;
    translate([-(aa_dia+aa_dist)/2, -aa_dia/2, aa_wall])
      cube([aa_dia+aa_dist, aa_dia, empty_space_fraction*height]);
    translate([-(aa_dia+aa_dist)/2, -aa_dia/2, height-aa_wall-empty_space_fraction*height])
      cube([aa_dia+aa_dist, aa_dia, empty_space_fraction*height]);

    translate([0, aa_dia/2+aa_wall-m3_head_len, height/2]) rotate([90, 0, 0]) m3_screw(len=aa_dia+1*aa_wall-m3_head_len, nut_at=10, nut_thick=40);
  }
}

// difference to remove the lid, intersection to get lid.
module battery_box_separator_block(lid_offset, depth,
				   slope_top, slope_bottom,
				   width, height_diff) {
  behind_cut=lid_offset + 10;
  inner_width=2*aa_dia + aa_dist;
  inner_height=aa_len;
  height=inner_height + 2*aa_wall + 2*e;

  keepout_inner_width=inner_width-2*e;
  keepout_inner_height=inner_height-2*e;
  keepout_inner_thick=aa_dia+aa_wall;

  thick=aa_dia+2*aa_wall;

  translate([-width/2, 0, 0])
    rotate([90, 0, 90])
    linear_extrude(height=width+e)
    polygon([ [0, height_diff-e], [behind_cut, height_diff-e],
              [behind_cut, height-height_diff+e],
              [0, height-height_diff+e],
              [0, height-aa_wall],
              [-depth, height - slope_top*depth],  // first corner
              [-depth, slope_bottom * depth],  // second corner
              [0, aa_wall],
              ]);
}

// Separate lid from the battery box
module battery_box_separator(is_inside=false, lid_offset=5, depth=5,
			     slope_top=1.1, slope_bottom=2.3,
			     align_rim_deep=battery_box_rim_deep) {
  behind_cut=lid_offset + 10;
  inner_width=2*aa_dia + aa_dist;
  inner_height=aa_len;
  height=inner_height + 2*aa_wall + 2*e;

  keepout_inner_width=inner_width-2*e;
  keepout_inner_height=inner_height-2*e;
  keepout_inner_thick=aa_dia+aa_wall;

  thick=aa_dia+2*aa_wall;

  translate([0, thick-lid_offset, 0]) {
    difference() {
      union() {
        battery_box_separator_block(lid_offset, depth,
                                    slope_top, slope_bottom,
                                    inner_width + 2*aa_wall + 2*e + 20,
                                    -10);
        translate([0, -align_rim_deep, 0])
          battery_box_separator_block(lid_offset, depth,
                                      slope_top, slope_bottom,
                                      (2*aa_dia + aa_dist + 1*aa_wall) + (is_inside ? -fit_tolerance/2 : fit_tolerance/2),
                                      aa_wall/2+(is_inside ? fit_tolerance/2 : -fit_tolerance/2));
      }

      translate([0, -battery_box_rim_deep, 0])
        translate([-keepout_inner_width/2, -keepout_inner_thick, aa_wall+e]) cube([keepout_inner_width, keepout_inner_thick, keepout_inner_height]);
    }
  }
}

// Punch for cables of batteries.
module battery_power_punch() {
  cable_radius=1.2;
  wide_factor=2;
  // First part is a straight round hole.
  hull() {
    translate([-aa_dist-8, aa_wall, aa_wall+cable_radius]) rotate([90, 0, 0]) scale([wide_factor, 1, 1]) cylinder(r=cable_radius, h=1);
    translate([-aa_dist-8, aa_wall+5, aa_wall+cable_radius]) rotate([90, 0, 0]) scale([wide_factor, 1, 1]) cylinder(r=cable_radius, h=1);
  }

  // Folled by a wedge that allows access from top.
  hull() {
    translate([-aa_dist-8, aa_wall, aa_wall+cable_radius]) rotate([90, 0, 0])
      scale([wide_factor, 1, 1]) cylinder(r=cable_radius, h=1);
    translate([-cable_channel_distance-cable_channel_front_wide, -dial_thick-1, cable_channel_from_bottom]) cube([cable_channel_front_wide, 1, stem_high+20]);
  }
}

// Mounting holes to hold down spheormeter frame.
module bottom_screw_punch() {
  screw_len=stem_high * 2.3;

  // Back
  translate([0, bottom_mount_back_offset, 0]) {
    translate([bottom_mount_back_distance/2, 0, 0]) rotate([0, 0, 360 - 150]) m3_screw(len=screw_len, nut_at=stem_high, nut_thick=18);
    translate([-bottom_mount_back_distance/2, 0, 0]) rotate([0, 0, 150]) m3_screw(len=screw_len, nut_at=stem_high, nut_thick=18);
  }

  // Front
  translate([0, -bottom_mount_front_offset, 0]) {
    translate([bottom_mount_front_distance/2, 0, 0]) m3_screw(len=screw_len, nut_at=stem_high, nut_channel=bottom_mount_front_offset);
    translate([-bottom_mount_front_distance/2, 0, 0]) m3_screw(len=screw_len, nut_at=stem_high, nut_channel=bottom_mount_front_offset);
  }

  // Hole through the squeeze block. Optional.
  if (squeeze_block_mount) {
    translate([0, -bottom_mount_center_offset, 0]) m3_screw(len=screw_len, nut_at=stem_high/2, nut_channel=bottom_mount_front_offset);
  }
}

// Screws needed to hold the stem in place
module stem_squeeze_block_punch() {
  mount_meat = 8;  // The 'meat' before we hit the butt-surface
  mount_screw_len = dial_thick - dial_stem_pos + mount_meat + aa_wall/2;
  translate([stem_mount_screw_distance/2, -mount_meat, max(m3_head_dia/2+1, stem_high/2)])
    rotate([-90, 0, 0]) m3_screw(len=mount_screw_len,
                                 nut_at=mount_meat+bottom_mount_back_offset-m3_nut_dia+m3_nut_thick, nut_channel=2*stem_high);
  translate([-stem_mount_screw_distance/2, -mount_meat, max(m3_head_dia/2+1, stem_high/2)])
    rotate([-90, 0, 0]) m3_screw(len=mount_screw_len,
                                 nut_at=mount_meat+bottom_mount_back_offset-m3_nut_dia+m3_nut_thick, nut_channel=2*stem_high);
}

// Frame of the spheormeter with all solid parts put together and negative
// spaces punched out. This part will then be separated in
// stem_squeeze_block, back and battery lid below.
module spherometer_frame(cable_slots=true) {
  difference() {
    union() {
      hull() {
        base();
        dial_holder();
        translate([0, dial_thick - dial_stem_pos, 0]) battery_box(with_wiggle=false);
      }
      translate([0, dial_thick - dial_stem_pos, 0]) battery_box(with_wiggle=battery_box_with_wiggle);
    }
    dial_holder_punch(cable_slots);
    bottom_screw_punch();
    stem_squeeze_block_punch();
    translate([0, dial_thick - dial_stem_pos, 0]) {
      battery_box_punch();
      if (cable_slots) battery_power_punch();
    }
  }
}

module spherometer_frame_battery_lid() {
  intersection() {
    spherometer_frame();
    translate([0, dial_thick - dial_stem_pos + fit_tolerance, 0])
      battery_box_separator(is_inside=true);
    // Make lid a tiny bit shorter at the bottom, so that it fits
    // comfortably on the screwed-down case.
    translate([0, 0, 100+0.5]) cube([200, 200, 200], center=true);
  }
}

module spherometer_frame_main_block() {
  difference() {
    spherometer_frame();
    stem_squeeze_block_separator(is_inside=false);
    translate([0, dial_thick - dial_stem_pos, 0])
      battery_box_separator(is_inside=false);
  }
}

// Separating behind and front of dial.
module stem_squeeze_block_separator(is_inside=false) {
  w=squeeze_block_wide - (is_inside ? 2*fit_tolerance : 0);
  translate([-w/2, -100, -e]) cube([w, 100, stem_high+dial_wall+8]);
}

module spherometer_frame_stem_squeeze_block() {
  intersection() {
    spherometer_frame();
    translate([0, -fit_tolerance, -fit_tolerance]) stem_squeeze_block_separator(is_inside=true);
  }
}

// --
// Display. The display is separate from the main spherometer_frame and
// is 'plugigng' mechanically into a scrw punch channel and in the front
// is screwed down.
// --

module spherometer_frame_display_contact_cross_section() {
  intersection() {
    spherometer_frame(cable_slots=false);
    w=e;
    h=stem_high+dial_dia;
    dd=base_dia/2;
    translate([-50, -dd, -e]) cube([100, w, stem_high+dial_wall+8]);
  }
}

// The part of the display case that butts against the display case.
module display_top_block() {
  translate([-display_wide/2, -base_dia/2-display_transition, 0])
    cube([display_wide, display_transition, display_box_thick]);
}

// Transition between dial case and the actual box.
module display_transition_block() {
  slice=0.25;        // size of block slice.
  // We hull together many slices from the rounded bottom part of the
  // dial with square block of the display, blending these together.
  for (i = [-display_wide/2:slice:display_wide/2]) {
    hull() intersection() {
      translate([i, 0, 0]) cube([slice, 100, 100], center=true);
      union() {
        spherometer_frame_display_contact_cross_section();
        display_top_block();
      }
    }
  }

  // Where the spherometer frame hangs over the display width, we need
  // to transition that as well.
  for (i = [-display_wide/2, display_wide/2]) {
    hull() intersection() {
      translate([i + ((i<0) ? -5+e : 5-e), 0, 0]) cube([10, 100, 100], center=true);
      union() {
        spherometer_frame_display_contact_cross_section();
        display_top_block();
      }
    }
  }
}

module display_base_block() {
  start_y = -base_dia/2;
  // TODO: there is similar stuff going on in display-case
  have_wall = true;  // Separating wall. A little more rigidity if true.
  local_offset = have_wall ? 1 : -2*e;

  hull() {
    display_top_block();
    translate([-display_wide/2, start_y - display_high, 0]) champfer_point_cloud();
    translate([display_wide/2, start_y - display_high, 0]) scale([-1, 1, 1]) champfer_point_cloud();
    intersection() {  // Make front align smoothly with round bottom plate
      translate([0, mit_front - mit_front_adjust, 0]) cylinder(r=leg_plate_radius+display_front_radius_adjust, h=e);
      translate([-display_wide/2, -100-base_dia/2, 0]) cube([display_wide, 100, 1]);
    }
  }

  // Little tactile extrusion to 'feel' the button.
  tactile_bobble_r=2.8;
  tactile_bobble_expose=1.5;
  translate([display_wide/2-tactile_bobble_r+tactile_bobble_expose, start_y-display_button_from_top-local_offset, 0.4*display_box_thick]) sphere(r=tactile_bobble_r, $fn=30);

  // Retaining blocks to be matched up with the holes for the M3 nuts of the
  // device.
  if (false) {
    nut_wide=m3_nut_dia * cos(30) - 2*fit_tolerance;
    for (offset = [-bottom_mount_front_distance/2, bottom_mount_front_distance/2]) {
      translate([-nut_wide/2 + offset, -base_dia/2-e, stem_high+fit_tolerance]) cube([nut_wide, 3, m3_nut_thick-2*fit_tolerance]);
    }
  }
}

module rounded_corner_box(coords=[10,10,10], r=1) {
  hull() {
    translate([0+r, 0+r, 0]) cylinder(r=r, h=coords[2]);
    translate([coords[0]-r, 0+r, 0]) cylinder(r=r, h=coords[2]);
    translate([coords[0]-r, coords[1]-r, 0]) cylinder(r=r, h=coords[2]);
    translate([0+r, coords[1]-r, 0]) cylinder(r=r, h=coords[2]);
  }
}

// The electronics and display. Space it needs inside its casing.
// Poking through top (view display and button) and bottom.
module display_electronics_punch() {
  extra=0.2;
  w=35.7+2*extra;
  h=33.4+2*extra;
  knob_long=0.5;
  knob_poke_wide=2.5;
  knob_poke_high=6;   // Height of the button hitting the pcb switch.
  knob_from_top=display_button_from_top;
  translate([-w/2, -h, -display_box_thick+e]) difference() {
    union() {
      cube([w, h, display_box_thick]);  // PCB size
      translate([w, h-knob_from_top-10/2, 0]) cube([knob_long, 10, display_box_thick-0]);
    }
    translate([w-knob_long+e, h-knob_from_top-knob_poke_wide/2, 0]) cube([knob_long+5, knob_poke_wide, knob_poke_high]);
  }

  // Display
  dw=31;
  dh=16.5;
  translate([-dw/2, -dh-6.5, -e]) color("blue") rounded_corner_box([dw, dh, 10], r=2);

  // old switch
  if (false) translate([w/2-e, -h+6, -10]) {
    translate([0, -4.5, 0]) hull() {
      cube([5, 8, 7]);
      translate([0, 15, 0]) cube([e, e, 7]);
    }
    translate([5-e, 0, 3.5]) rotate([0, 90, 0]) cylinder(r=4/2, h=4);
  }
}

module display_case_punch() {
  have_wall = true;  // Separating wall. A little more rigidity if true.
  local_offset = have_wall ? 1 : -2*e;
  translate([0, -base_dia/2 - local_offset, display_box_thick-display_top_wall]) display_electronics_punch();

  // Screw at the very front to hold down display.
  if (false) {
    translate([0, -bottom_mount_front_display_center_offset, 0]) rotate([0, 0, 150]) m3_screw(len=display_box_thick-3, nut_at=1, nut_channel=10);
  }
}

module display_cable_channel_punch(at_x, wide) {
  translate([at_x, e, -e]) hull() {
    translate([-wide/2, -base_dia/2-display_transition,0]) cube([wide, e, e]);
    translate([-wide/2, -base_dia/2, 0]) cube([wide, e, stem_high+5]);
  }
}

module display_case() {
  difference() {
    union() {
      if (false) display_transition_block();  // From round to square
      display_base_block();        // Rest of the box.
    }
    display_case_punch();

    if (false) {
      // Channels for the cable
      w = cable_channel_front_wide;
      display_cable_channel_punch(cable_channel_distance+w/2, w);
      display_cable_channel_punch(-cable_channel_distance-w/2, w);
    }
  }
}

module display_case_button() {
  // TODO: there is similar stuff going on in display-case
  have_wall = true;  // Separating wall. A little more rigidity if true.
  local_offset = have_wall ? 1 : -2*e;

  intersection() {
    display_case();
    translate([display_wide/2+e,-base_dia/2 - local_offset -display_button_from_top, display_top_wall+e]) display_button_separator();
  }
}

module display_part() {
  // TODO: there is similar stuff going on in display-case
  have_wall = true;  // Separating wall. A little more rigidity if true.
  local_offset = have_wall ? 1 : -2*e;

  difference() {
    display_case();
    translate([display_wide/2+e,-base_dia/2 - local_offset -display_button_from_top, display_top_wall+e]) display_button_separator(extra=0.3, actuation=0.8);
  }
}

// Separating display box in two separately printable parts.
module display_separator() {
  translate([0, 0, 25+display_box_thick]) cube([100, base_dia+2*display_transition, 50], center=true);
}

// Printable: front part can be printed flat, upside down, which leaves a
// nice surface.
module display_front_part() {
  difference() { display_part(); display_separator(); }
}

// The curved transition part can be printed separately, then glued on top
// of the display_front_part.
module display_transition_part() {
  intersection() { display_part(); display_separator(); }
}

module leg_plate() {
  difference() {
    color("#f0f0ff", alpha=0.3) cylinder(r=leg_plate_radius, h=leg_plate_thick);
    // TODO: unite hole pattern here and bottom_screw_punch()
    translate([0, bottom_mount_back_offset, -e]) {
      translate([bottom_mount_back_distance/2, 0, 0]) cylinder(r=m3_dia/2, h=leg_plate_thick+2*e);
      translate([-bottom_mount_back_distance/2, 0, 0]) cylinder(r=m3_dia/2, h=leg_plate_thick+2*e);
    }
    translate([0, -bottom_mount_front_offset, -e]) {
      translate([bottom_mount_front_distance/2, 0, 0]) cylinder(r=m3_dia/2, h=leg_plate_thick+2*e);
      translate([-bottom_mount_front_distance/2, 0, 0]) cylinder(r=m3_dia/2, h=leg_plate_thick+2*e);
    }
    if (squeeze_block_mount) {
      translate([0, -bottom_mount_center_offset, -e]) cylinder(r=m3_dia/2, h=leg_plate_thick+2*e);
    }
    translate([0, -bottom_mount_front_display_center_offset, -e]) {
      cylinder(r=m3_dia/2, h=leg_plate_thick+2*e);
    }

    //translate([0, 0, -1]) bottom_screw_punch();
    translate([0, 0, -e]) cylinder(r=stem_dia/2, h=leg_plate_thick+2*e);
    for (r = [0, 120, 240]) {
      rotate([0, 0, r-30])
        translate([leg_radius, 0, -e]) cylinder(r=leg_ball_hole_dia/2, h=leg_plate_thick+2*e);
    }
  }
}

module leg_plate_2d() {  // laser cut this.
  projection(cut=true) leg_plate();
}

module demo_leg_balls() {
  ball_r=leg_ball_dia/2;
  hole_r=leg_ball_hole_dia/2;
  pos = ball_r - sqrt(ball_r*ball_r - hole_r*hole_r);
  for (r = [0, 120, 240]) {
    rotate([0, 0, r-30])
      color("silver") translate([leg_radius, 0, -ball_r+pos]) sphere(r=leg_ball_dia/2);
  }
}

module demo_leg_plate() {
  translate([0, 0, -leg_plate_thick]) {
    %leg_plate();   // 'transparent'
    demo_leg_balls();
  }
}

module display_button_separator(extra=0, actuation=0) {
  knob_y=5 + 2*extra;
  //deep = (display_wide - 36) / 2 + 1 + extra;
  deep = 3 + actuation;
  translate([-deep, -knob_y/2, -10]) {
    hull() {
      cube([deep+5, knob_y, display_box_thick+10]);  // button face to outside
      translate([-3, knob_y/2, 0]) cylinder(r=0.5, h=display_box_thick+10);
    }
  }
  // cone to hold it in place
  translate([0, 0, 0]) hull() {
    translate([-1, -knob_y/2, -2+extra]) cube([0.2, knob_y, display_box_thick]);
    translate([-10, -(knob_y+5)/2, -2+extra]) cube([0.2, knob_y+5, display_box_thick]);
  }
}

// Assemble of the autolet to see how that looks.
module assembly_autolet() {
  demo_leg_plate();
  color("red") render() spherometer_frame_stem_squeeze_block();
  color("yellow") render() spherometer_frame_main_block();
  color("blue") translate([0, 0*20, 0]) render() spherometer_frame_battery_lid();
  translate([0, -0*10, 0]) {
    color("gray") render() display_part();
    color("yellow") render() display_case_button();
  }
}

module mit_backend() {
  color("yellow") render() difference() {
    battery_box();
    battery_box_separator(is_inside=false);
  }
}


module mit_battery_cover() {
  color("blue")
    render() intersection() {
    battery_box(with_wiggle=battery_box_with_wiggle);
    battery_box_separator(is_inside=true);
  }
}

mit_back=7.5 - 1.0;
mit_front=22;
mit_front_adjust=18.4;
mit_dia=18.25;
mit_bottom_dia=23;
mit_wide=display_wide;
mit_conn_bar_thick=14;

module mit_center_block(h=mit_conn_bar_thick) {
  back_wide = 44.2;  // about battery box
  hull() {
    translate([-mit_wide/2, -mit_front, 0]) cube([mit_wide, mit_front, h]);
    translate([-back_wide/2, 0, 0]) cube([back_wide, mit_back, h]);
  }
}

module mit_center_punch(h=mit_conn_bar_thick) {
  translate([0, 0, -e]) cylinder(r=mit_dia/2, h=max(15, h+2*e));
  scale([0.8, 1.2, 1]) translate([0, 0, -e]) cylinder(r=mit_dia/2, h=max(14, h+2*e));
  translate([0, 0, -e]) cylinder(r=mit_bottom_dia/2, h=4);
  translate([0, 0, -e]) cylinder(r1=26/2, r2=mit_bottom_dia/2, h=2);  // accomodate glue ridge.
  translate([0, 0, 9]) rotate([-90, 0, 0]) cylinder(r=4/2, h=35);  // Grub screw access
}

module mit_power_punch() {
  front = mit_front;
  back = mit_back+8;
  wide = 3;
  high = 3.5;
  translate([-mit_wide/2+8, 0, 0]) {
    hull() {
      translate([0, -front+6, -e]) cylinder(r=wide/2, h=high);
      translate([0, back, -e]) cylinder(r=wide/2, h=high);
    }
    hull() {
      translate([0, -front+6, -e]) cylinder(r=wide/2, h=high);
      translate([3, -front-3, -e]) cylinder(r=wide, h=high);
    }
  }
  //translate([-mit_wide/2 + 2, -front, -e]) cube([3, front+back, 4]);
}

module mit_display_wedge(extra_front_len=0) {
  hull() {
    translate([-display_wide/2, -mit_front+1 - extra_front_len, 0]) cube([display_wide, 0.1+extra_front_len, display_box_thick]);
    translate([0, 0, 1]) cube([mit_wide, 1, 2], center=true);
  }
}

module mit_display_data_cable_punch() {
  hull() {
    translate([18, -mit_front, 2.5]) sphere(r=2.5);
    translate([mit_wide/2+2, -mit_front+15, 12]) sphere(r=2.2);
  }
}

module mit_battery_cable_punch() {
  w=mit_wide+2*e;
  translate([0, mit_back-0.5, mit_conn_bar_thick+2]) rotate([0, 90, 0]) translate([0, 0, -w/2]) cylinder(r=4.5/2, h=w);
}

module mit_frame() {
  difference() {
    union() {
      translate([0, mit_back, 0]) mit_backend();  // battery box, essentially.

      mit_center_block();

      translate([0, -mit_front+mit_front_adjust, 0]) display_part();
      mit_display_wedge();
    }
    mit_center_punch();
    mit_power_punch();  // power cable channel
    mit_display_data_cable_punch();  // cable going into display
    mit_battery_cable_punch();

    // Screws
    translate([-(mit_dia/2 + 4), -mit_front+3.3, mit_conn_bar_thick-3.2]) rotate([90, 0, 180]) m3_screw(len=27.5, nut_at=21, nut_channel=15);
    translate([+(mit_dia/2 + 4), -mit_front+3.3, mit_conn_bar_thick-3.2]) rotate([90, 0, 180]) m3_screw(len=27.5, nut_at=21, nut_channel=15);

    //translate([0, -leg_plate_radius+6, 0]) cylinder(r=4/2, h=3);  // front display down screw

    translate([-display_wide/2 + 4, -mit_front-25, 0]) #cylinder(r=4/2, h=5);
    translate([display_wide/2 - 4, -mit_front-25, 0]) #cylinder(r=4/2, h=5);
  }
}

module mit_front_back_separator(is_front=true) {
  separation_offset=0.4 / 2;
  translate([-50, -100-(is_front ? 1 : -1) * separation_offset, -10]) cube([100, 100, 100]);
}

module mit_front_part() {
  intersection() {
    mit_frame();
    mit_front_back_separator(is_front=true);
  }
}

module mit_back_part() {
  difference() {
    mit_frame();
    mit_front_back_separator(is_front=false);
  }
}

module mit_puzzle_wedge(h=display_box_thick, extra=0, big_r=4) {
  hull() {
    cylinder(r=big_r+extra, h=h);
    translate([0, -10, 0]) cylinder(r=0.2+extra, h=h);
  }
}

module mit_bar_display_separator(enlarge=false) {
  w = display_wide + 2*e;
  h = display_box_thick+3*e;
  enlarge_value = enlarge ? 0.1 : 0;
  translate([-w/2, -80 - mit_front+1+1 + enlarge_value, -e]) cube([w, 80, h+enlarge_value]);

  translate([-23, -mit_front+4+3, -e]) mit_puzzle_wedge(h=h, extra=enlarge_value, big_r=2);
  translate([-12, -mit_front+4+3, -e]) mit_puzzle_wedge(h=h, extra=enlarge_value);
  translate([+12, -mit_front+4+3, -e]) mit_puzzle_wedge(h=h, extra=enlarge_value);
  //translate([+23, -mit_front+4+3, -e]) mit_puzzle_wedge(h=h, extra=enlarge_value, big_r=2);
}

module mit_front_display_part() {
  intersection() {
    mit_front_part();
    mit_bar_display_separator(enlarge=false);
  }
}

module mit_front_block_part() {
  difference() {
    mit_front_part();
    mit_bar_display_separator(enlarge=true);
  }
}

//mit_frame();
//display_base_block();
//display_case();
//translate([0, 15, 0]) mit_battery_cover();
if (true) {
color("yellow") mit_back_part();
translate([0, -mit_front+mit_front_adjust, 0])  color("yellow") render() display_case_button();
color("gray") render() mit_front_display_part();
color("red") render() mit_front_block_part();
color("azure", 0.2) translate([0, 0, -2-e]) cylinder(r=leg_plate_radius, h=2);
}

//display_case_button();
//display_case();
//display_electronics_punch();

//assembly_autolet();