// Mounting frame holding the indicator and display
// CAVE: Super-hacky right now while exploring different shape.
//
// Rough nomenclature:
// Parts often come in two modules: foo() and foo_punch(). foo() defines the
// outer size, while foo_punch() the negative space. They are separate as often
// negative spaces are applied after part is built up.
//
// Some parts also come with a foo_separator(). This defines some cut through
// a part (somtimes with a complicated shape), which separates a larger
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
m3_nut_dia=5.4 / cos(30) + 2*fit_tolerance;  // div by cos(30) for outer circle
m3_nut_thick=2.8;

// Sperometer legs.
leg_radius=50;
leg_plate_thick=12.7;    // Thickness of the acrylic used.
leg_ball_dia=12.7;
leg_ball_hole_dia=8;
leg_plate_rim=5;         // Extra acrylic beyond the legs.
leg_plate_radius=leg_radius + leg_ball_hole_dia/2 + leg_plate_rim;

dial_dia=57.5;
dial_cable_thick=1.5;   // Thickness of the data cables.
dial_wall=dial_cable_thick+1;
dial_thick=25.2;
dial_stem_pos = 21.4-4;  // Position of stem from the frontface
dial_cable_pos=12;   // Position of the cable channel from the front

// Parameters from the autolet indicator
stem_dia=8;                       // Stem of the meter
stem_bushing_len=21.5;            // How long is the stem bushing
stem_high=stem_bushing_len - leg_plate_thick - dial_wall;
stem_mount_screw_distance=stem_dia + 8;

// Battery sizes
aa_dia=14.5 + 2*fit_tolerance;
aa_len=50.5 + 8;  // for contacts
aa_wall=3;
aa_dist = 8;      // Distance between batteries

// Battery box wiggle printing on the back is a challenge. Maybe this needs to
// be split in the wiggle part and flat battery box back, that are then glued
// together.
battery_box_with_wiggle=false;   // Easier to print if false :)
battery_box_rim_deep=1.5;       // Alignment rim all around battery box.

base_dia=dial_stem_pos*2;

display_wall_thick=1.5;
display_wide=55;
display_high=35;
display_front_radius=5;

// Mounting holes, holding down the back part, the front part and the
// display part.
bottom_mount_front_offset=10;  // Bottom screws. Offset from center to back.
bottom_mount_center_offset=bottom_mount_front_offset + 0;
bottom_mount_front_distance=display_wide - 8;  // right/left distance.

bottom_mount_front_display_offset=display_high+base_dia/2-display_wall_thick-display_front_radius;
bottom_mount_front_display_distance=display_wide - 2*display_wall_thick - 2*display_front_radius;

bottom_mount_back_offset=7;  // Bottom screws. Offset from center to back.
bottom_mount_back_distance=display_wide - 13;  // right/left distance.

function max(a, b) = a > b ? a : b;

// m3_screw: module to punch material.
// nut_at: star of where a m3 nut shoud be placed. -1 for off.
// nut_channel: make a channel of given length to slide a nut in.
module m3_screw(len=60, nut_at=-1, nut_channel=-1, nut_thick=m3_nut_thick) {
     cylinder(r=m3_dia/2, h=len);
     translate([0, 0, -20+e]) cylinder(r=m3_head_dia/2, h=20);
     if (nut_at > 0) {
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

module stem_punch() {
     // A little thinner in y direction to have enough 'squeeze' action.
     scale([1, 0.98, 1]) translate([0, 0, -25]) cylinder(r=stem_dia/2, h=50);
}

module stem_holder() {
     cylinder(r=stem_dia/2 + 4, h=stem_high);
}

// Base where everything is sitting on the bottom.
module base(with_front_flat=true) {
     scale([1, 1, 1]) cylinder(r=base_dia/2, h=0.8);
     translate([-display_wide/2, -base_dia/2, 0]) cube([display_wide, base_dia/2, 2]);
     if (with_front_flat) {
	  translate([-display_wide/2, -base_dia/2, 0]) cube([display_wide, 1, stem_high]);
     }
}

module version_punch(thick=1) {
     fs=5;
     color("red") linear_extrude(height=thick) {
	  text("© H. Zeller", halign="center", size=fs);
	  translate([0, -1*(fs+1)]) text(version_date, halign="center", size=fs);
	  translate([0, -2*(fs+1)]) text(version_id, halign="center", size=fs);
     }
}

module dial_punch(cable_slot=true) {
     extra=40;
     cable_management_channel=dial_wall/2;
     wall_r =dial_dia/2 + dial_wall;
     connector_wide=7;
     connector_from_top=13;

     connector_angle_from_top=90
	  + 360 * (-connector_wide - connector_from_top) / (dial_dia * PI);
     connector_angle_width=360 * connector_wide / (dial_dia * PI);
     rotate([90, 0, 0])
	  translate([0, wall_r+stem_high, -dial_thick+dial_stem_pos]) {

	  // The dial. With extra punch to the front.
	  cylinder(r=dial_dia/2, h=dial_thick+extra);
	  translate([-7, 12, -0.5]) version_punch(0.5);

	  // Leave the top free.
	  top_punch=connector_from_top;
	  translate([-top_punch,0, 0]) cube([2*top_punch, dial_dia, dial_thick]);
     }
     stem_punch();

     // More punch for cable management: lead it out on the back into the slot
     if (cable_slot) color("red") hull() {
	       bottom_wide=connector_wide * 0.8;
	       translate([stem_mount_screw_distance, -dial_stem_pos-e, dial_wall+stem_high/2]) cube([3, 1, stem_high+10]);
	       translate([stem_mount_screw_distance-bottom_wide/2, dial_thick-dial_stem_pos, dial_wall+7]) cube([bottom_wide, dial_cable_thick, dial_cable_thick]);
	       rotate([90, 0, 0])
		    translate([0, wall_r+stem_high, -dial_thick+dial_stem_pos])
		    translate([0, 0, -dial_cable_thick]) rotate([0, 0, connector_angle_from_top]) rotate_extrude(angle=connector_angle_width, convexity=2) translate([dial_dia/2, 0, 0]) square([dial_cable_thick, dial_thick+dial_cable_thick]);
	  }
}

module dial_holder() {
     wall_r =dial_dia/2 + dial_wall;
     rotate([90, 0, 0]) translate([0, wall_r+stem_high, -dial_thick+dial_stem_pos]) cylinder(r=wall_r, h=dial_thick);
}

module aa_punch(h=3) {
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

module aabat(punch=false, straight_cut=1) {
     translate([0, 0, -aa_len/2]) {
	  cylinder(r=aa_dia/2, h=aa_len);
	  translate([-aa_dia/2, 0, 0]) cube([aa_dia, aa_dia/2-straight_cut, aa_len]);
	  translate([0, -aa_dia/2, aa_len/2]) rotate([90, 0, 0]) aa_punch();
     }
}

module battery_box_punch() {
     height=aa_len + 2*aa_wall;
     width=2*aa_dia + aa_dist + 2*aa_wall;
     thick=aa_dia+2*aa_wall;

     translate([0, thick/2, 0]) {
	  translate([-(aa_dia+aa_dist)/2, 0, aa_wall+aa_len/2]) aabat();
	  translate([+(aa_dia+aa_dist)/2, 0, aa_wall+aa_len/2]) rotate([0, 180, 0]) aabat();

	  empty_space_fraction=0.0;
	  translate([-(aa_dia+aa_dist)/2, -aa_dia/2, aa_wall])
	       cube([aa_dia+aa_dist, aa_dia, empty_space_fraction*height]);
	  translate([-(aa_dia+aa_dist)/2, -aa_dia/2, height-aa_wall-empty_space_fraction*height])
	       cube([aa_dia+aa_dist, aa_dia, empty_space_fraction*height]);

	  translate([0, aa_dia/2+aa_wall-m3_head_len, height/2]) rotate([90, 0, 0]) m3_screw(len=aa_dia+1*aa_wall-m3_head_len, nut_at=5, nut_thick=40);
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

module sine_wiggle(tau_coverage=3, len=20, height=2, resolution=0.01) {
     points = [ for (w = [0 : resolution : 1.0]) [ w * len, height/2 + height/2 * sin(w*tau_coverage*360-90)] ];
     points1=concat([[0, -e]], points, [[len, -e]]);
     translate([-len/2, 0, 0]) polygon(points1);
}

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

module battery_power_punch() {
     cable_radius=1.1;
     // First part is a straight round hole.
     hull() {
     	  translate([-aa_dist-8, aa_wall, aa_wall+cable_radius]) rotate([90, 0, 0]) cylinder(r=cable_radius, h=1);
	  translate([-aa_dist-8, aa_wall+5, aa_wall+cable_radius]) rotate([90, 0, 0]) cylinder(r=cable_radius, h=1);
     }
     // Folled by a wedge that allows access from top.
     hull() {
	  translate([-aa_dist-8, aa_wall, aa_wall+cable_radius]) rotate([90, 0, 0])
	       cylinder(r=cable_radius, h=1);
	  translate([-stem_mount_screw_distance-cable_radius, -dial_thick-1, stem_high/2+aa_wall]) cube([3, 1, stem_high+35]);
     }
}

module bottom_screw_punch() {
     screw_len=stem_high * 2.3;

     // Back
     translate([0, bottom_mount_back_offset, 0]) {
	  translate([bottom_mount_back_distance/2, 0, 0]) rotate([0, 0, 360 - 150]) m3_screw(len=screw_len, nut_at=stem_high, nut_channel=15);
	  translate([-bottom_mount_back_distance/2, 0, 0]) rotate([0, 0, 150]) m3_screw(len=screw_len, nut_at=stem_high, nut_channel=15);
     }

     // Front
     translate([0, -bottom_mount_front_offset, 0]) {
	  translate([bottom_mount_front_distance/2, 0, 0]) m3_screw(len=screw_len, nut_at=stem_high, nut_channel=bottom_mount_front_offset);
	  translate([-bottom_mount_front_distance/2, 0, 0]) m3_screw(len=screw_len, nut_at=stem_high, nut_channel=bottom_mount_front_offset);
     }
     translate([0, -bottom_mount_center_offset, 0]) m3_screw(len=screw_len, nut_at=stem_high/2, nut_channel=bottom_mount_front_offset);

     // Very front to hold down display.
     translate([0, -bottom_mount_front_display_offset, 0]) {
	  translate([bottom_mount_front_display_distance/2, 0, 0]) m3_screw(len=screw_len, nut_at=stem_high);
	  translate([-bottom_mount_front_display_distance/2, 0, 0]) m3_screw(len=screw_len, nut_at=stem_high);
     }
}

module dial_case(cable_slots=true) {
     difference() {
	  union() {
	       hull() {
		    base();
		    stem_holder();
		    dial_holder();
		    translate([0, dial_thick - dial_stem_pos, 0]) battery_box(with_wiggle=false);
	       }
	       translate([0, dial_thick - dial_stem_pos, 0]) battery_box(with_wiggle=battery_box_with_wiggle);
	  }
	  dial_punch(cable_slots);
	  bottom_screw_punch();

	  // Stem holding. This should be a separate module.
	  mount_meat = 8;  // The 'meat' before we hit the butt-surface
	  mount_screw_len = dial_thick - dial_stem_pos + mount_meat + aa_wall/2;
	  translate([stem_mount_screw_distance/2, -mount_meat, max(m3_head_dia/2+1, stem_high/2)])
	       rotate([-90, 0, 0]) m3_screw(len=mount_screw_len,
					    nut_at=mount_meat+bottom_mount_back_offset-m3_nut_dia+m3_nut_thick, nut_channel=2*stem_high);
	  translate([-stem_mount_screw_distance/2, -mount_meat, max(m3_head_dia/2+1, stem_high/2)])
	       rotate([-90, 0, 0]) m3_screw(len=mount_screw_len,
					    nut_at=mount_meat+bottom_mount_back_offset-m3_nut_dia+m3_nut_thick, nut_channel=2*stem_high);
	  translate([0, dial_thick - dial_stem_pos, 0]) {
	       battery_box_punch();
	       if (cable_slots) battery_power_punch();
	  }
     }
}

// Separating behind and front of dial.
module dial_separator(is_inside=false) {
     // TODO: calculate base-width from other values.
     w=32 - (is_inside ? 2*fit_tolerance : 0);
     translate([-w/2, -100, -e]) cube([w, 100, stem_high+dial_wall+8]);
}

module dial_battery_lid() {
     intersection() {
	  dial_case();
	  translate([0, dial_thick - dial_stem_pos + fit_tolerance, 0])
	       battery_box_separator(is_inside=true);
	  // Make lid a tiny bit shorter at the bottom, so that it fits
	  // comfortably on the screwed-down case.
	  translate([0, 0, 100+0.5]) cube([200, 200, 200], center=true);
     }
}

module dial_backend() {
     difference() {
	  dial_case();
	  dial_separator(is_inside=false);
	  translate([0, dial_thick - dial_stem_pos, 0])
	       battery_box_separator(is_inside=false);
     }
}

module dial_frontend() {
     intersection() {
	  dial_case();
	  translate([0, -fit_tolerance, -fit_tolerance]) dial_separator(is_inside=true);
     }
}

module leg_plate() {
     difference() {
	  color("#f0f0ff", alpha=0.3) cylinder(r=leg_plate_radius, h=leg_plate_thick);
	  // TODO: make hole pattern here and bottom_screw_punch()
	  // Hole pattern
	  translate([0, bottom_mount_back_offset, -e]) {
	       translate([bottom_mount_back_distance/2, 0, 0]) cylinder(r=m3_dia/2, h=leg_plate_thick+2*e);
	       translate([-bottom_mount_back_distance/2, 0, 0]) cylinder(r=m3_dia/2, h=leg_plate_thick+2*e);
	  }
	  translate([0, -bottom_mount_front_offset, -e]) {
	       translate([bottom_mount_front_distance/2, 0, 0]) cylinder(r=m3_dia/2, h=leg_plate_thick+2*e);
	       translate([-bottom_mount_front_distance/2, 0, 0]) cylinder(r=m3_dia/2, h=leg_plate_thick+2*e);
	  }
	  translate([0, -bottom_mount_center_offset, -e]) cylinder(r=m3_dia/2, h=leg_plate_thick+2*e);
	  translate([0, -bottom_mount_front_display_offset, -e]) {
	       translate([bottom_mount_front_display_distance/2, 0, 0]) cylinder(r=m3_dia/2, h=leg_plate_thick+2*e);
	       translate([-bottom_mount_front_display_distance/2, 0, 0]) cylinder(r=m3_dia/2, h=leg_plate_thick+2*e);
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

module leg_balls() {
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
	  %leg_plate();
	  leg_balls();
     }
}

// Assemble to see how that looks.
if (true) {
     demo_leg_plate();
     color("red") render() dial_frontend();
     color("yellow") render() dial_backend();
     color("blue") translate([0, 1*20, 0]) render() dial_battery_lid();
}
