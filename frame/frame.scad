// Mounting frame holding the indicator and display
// CAVE: Super-hacky right now while exploring different shape.

print_quality=true;   // print quality: high-res, but slow to render.
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
dial_wall=2;
dial_thick=25.2;
dial_stem_pos = 21.4-4;  // Position of stem from the frontface
dial_cable_pos=12;   // Position of the cable channel from the front

// Stem of the meter
stem_dia=8;
stem_bushing_len=21.5;
stem_high=stem_bushing_len - leg_plate_thick;
stem_mount_screw_distance=stem_dia + 8;

// Battery sizes
aa_dia=14.5 + 2*fit_tolerance;
aa_len=50.5 + 8;  // for contacts
aa_wall=2;
aa_dist = 8;      // Distance between batteries

base_dia=dial_stem_pos*2;

display_wall_thick=1.5;
display_wide=55;
display_high=35;
display_front_radius=5;

// Mounting holes, holding down the back part, the front part and the
// display part.
bottom_mount_front_offset=15;  // Bottom screws. Offset from center to back.
bottom_mount_front_distance=display_wide - 8;  // right/left distance.

bottom_mount_front_display_offset=display_high+base_dia/2-display_wall_thick-display_front_radius;
bottom_mount_front_display_distance=display_wide - 2*display_wall_thick - 2*display_front_radius;

bottom_mount_back_offset=7;  // Bottom screws. Offset from center to back.
bottom_mount_back_distance=display_wide - 13;  // right/left distance.

echo("back-distance: ", bottom_mount_back_distance,
     "offset: ", -bottom_mount_back_offset);
echo("middle-distance: ", bottom_mount_front_distance,
     "offset: ", bottom_mount_front_offset);
echo("display-distance: ", bottom_mount_front_display_distance,
     "offset: ", bottom_mount_front_display_offset);

// slit_nut: make a space to slide a nut in while printing.
module m3_screw(len=60, nut_at=-1, slit_nut=false) {
     cylinder(r=m3_dia/2, h=len);
     translate([0, 0, -20+e]) cylinder(r=m3_head_dia/2, h=20);
     if (nut_at > 0) {
	  translate([0, 0, nut_at]) {
	       rotate([0, 0, 30]) cylinder(r=m3_nut_dia/2, h=m3_nut_thick, $fn=6);
	       nut_wide=m3_nut_dia * cos(30);
	       if (slit_nut)
		    translate([-nut_wide/2, -m3_nut_dia/2, 0]) cube([nut_wide, m3_nut_dia/2, m3_nut_thick]);
	  }
     }
}

module stem_punch() {
     // A little thinner in y direction to have enough 'squeeze' action.
     scale([1, 0.98, 1]) translate([0, 0, -25]) cylinder(r=stem_dia/2, h=50);
     //translate([-stem_dia/2, -20, -e]) cube([stem_dia, 20, 50]);
}

module stem_holder() {
     cylinder(r=stem_dia/2 + 4, h=stem_high);
}

module base(with_front_flat=true) {
     scale([1, 1, 1]) cylinder(r=base_dia/2, h=0.8);
     translate([-display_wide/2, -base_dia/2, 0]) cube([display_wide, base_dia/2, 2]);
     if (with_front_flat) {
	  translate([-display_wide/2, -base_dia/2, 0]) cube([display_wide, 1, stem_high]);
     }
}

module display_base() {
     base_high=display_wall_thick;
     dw=display_wide - 2*display_wall_thick;
     dh=display_high - display_wall_thick;
     translate([0, -base_dia/2, 0]) union() { // Move to the front.
	  translate([-dw/2+display_front_radius,
		     -dh+display_front_radius, 0])
	       cylinder(r=display_front_radius, h=base_high);
	  translate([+dw/2-display_front_radius,
		     -dh+display_front_radius, 0])
	       cylinder(r=display_front_radius, h=base_high);
	  translate([-dw/2, 0, 0]) cube([dw, e, base_high]);
     }
}

module dial_punch(cable_slot=true) {
     extra=40;
     cable_management_channel=dial_wall/2;
     wall_r =dial_dia/2 + dial_wall;
     rotate([90, 0, 0])
	  translate([0, wall_r+stem_high, -dial_thick+dial_stem_pos]) {

	  // The dial. With extra punch to the front.
	  cylinder(r=dial_dia/2, h=dial_thick+extra);

	  // Punch for cable management. Angled, so that it is FDM printable.
	  translate([0, 0, dial_thick-dial_cable_pos]) difference() {
	       cylinder(r1=dial_dia/2+cable_management_channel,
			r2=dial_dia/2, h=dial_cable_pos);
	       translate([-50+stem_dia, 0, 0]) cube([100, 100, 100], center=true);
	  }
     }
     stem_punch();

     // More punch for cable management: lead it out in a slot
     if (cable_slot) translate([stem_mount_screw_distance, -dial_stem_pos-1, dial_wall+stem_high/2]) cube([3, dial_cable_pos+1, stem_high+10]);
}

module dial_holder() {
     wall_r =dial_dia/2 + dial_wall;
     intersection() {
	  rotate([90, 0, 0]) translate([0, wall_r+stem_high, -dial_thick+dial_stem_pos]) {
	       cylinder(r=wall_r, h=dial_thick);
	  }
	  battery_box_height=aa_len + 2*aa_wall;
	  dial_top = dial_dia + stem_high + dial_wall;
	  translate([-50, -50, 0]) cube([100, 100, min(0.8 * dial_top, battery_box_height)]);
     }
}

module aa_punch() {
     color("red") {
	  translate([0, -aa_len*0.25-8, -3+0.5]) linear_extrude(height=3) text("–", halign="center");
	  translate([0, aa_len*0.25, -3+0.5]) linear_extrude(height=3) text("+", halign="center");
     }
}

module aabat(punch=false) {
     translate([0, 0, -aa_len/2]) {
	  cylinder(r=aa_dia/2, h=aa_len);
	  translate([-aa_dia/2, 0, 0]) cube([aa_dia, aa_dia/2, aa_len]);
	  rotate([0, 0, -10]) translate([0, -aa_dia/2, aa_len/2]) rotate([90, 0, 0]) aa_punch();
     }
}

module battery_box_punch() {
     height=aa_len + 2*aa_wall;
     width=2*aa_dia + aa_dist + 2*aa_wall;
     thick=aa_dia+2*aa_wall;

     translate([0, thick/2, 0]) {
	  translate([-(aa_dia+aa_dist)/2, 0, aa_wall+aa_len/2]) aabat();
	  translate([+(aa_dia+aa_dist)/2, 0, aa_wall+aa_len/2]) rotate([0, 180, 0]) aabat();

	  empty_space_fraction=0.05;
	  translate([-(aa_dia+aa_dist)/2, -aa_dia/2, aa_wall])
	       cube([aa_dia+aa_dist, aa_dia, empty_space_fraction*height]);
	  translate([-(aa_dia+aa_dist)/2, -aa_dia/2, height-aa_wall-empty_space_fraction*height])
	       cube([aa_dia+aa_dist, aa_dia, empty_space_fraction*height]);

	  translate([0, aa_dia/2+aa_wall-m3_head_len, height/2]) rotate([90, 0, 0]) m3_screw(len=aa_dia+1*aa_wall-m3_head_len, nut_at=5);
     }
}

// difference to remove the lid, intersection to get lid.
module battery_box_separator(lid_offset=5) {
     wedge=20;
     height=aa_len;
     width=2*aa_dia + aa_dist;
     thick=aa_dia+2*aa_wall;
     translate([0, thick/2-lid_offset, height/2+aa_wall]) {
	  hull() {
	       translate([0, aa_dia/2, 0]) cube([width, 1, height], center=true);
	       translate([0, aa_dia/2+wedge, 0]) cube([width+2*wedge, 1, height+2*wedge], center=true);
	  }
     }
}

module battery_box() {
     height=aa_len + 2*aa_wall;
     width=2*aa_dia + aa_dist + 2*aa_wall;
     thick=aa_dia+2*aa_wall;
     difference() {
	  translate([0, thick/2, 0])  translate([-width/2, -thick/2, 0])
	       cube([width, thick, height]);
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
     screw_len=stem_high * 1.3;
     translate([0, bottom_mount_back_offset, 0]) {
	  translate([bottom_mount_back_distance/2, 0, 0]) m3_screw(len=screw_len, nut_at=stem_high/2, slit_nut=true);
	  translate([-bottom_mount_back_distance/2, 0, 0]) m3_screw(len=screw_len, nut_at=stem_high/2, slit_nut=true);
     }
     translate([0, -bottom_mount_front_offset, 0]) {
	  translate([bottom_mount_front_distance/2, 0, 0]) m3_screw(len=screw_len, nut_at=stem_high/2, slit_nut=true);
	  translate([-bottom_mount_front_distance/2, 0, 0]) m3_screw(len=screw_len, nut_at=stem_high/2, slit_nut=true);
     }
     translate([0, -bottom_mount_front_display_offset, 0]) {
	  translate([bottom_mount_front_display_distance/2, 0, 0]) m3_screw(len=screw_len, nut_at=stem_high/2, slit_nut=true);
	  translate([-bottom_mount_front_display_distance/2, 0, 0]) m3_screw(len=screw_len, nut_at=stem_high/2, slit_nut=true);
     }
}

module dial_case(cable_slots=true) {
     difference() {
	  union() {
	       hull() {
		    base();
		    stem_holder();
		    dial_holder();
		    translate([0, dial_thick - dial_stem_pos, 0]) battery_box();
	       }
	       hull() {
		    base(with_front_flat=false);
		    display_base();
	       }
	  }
	  dial_punch(cable_slots);
	  bottom_screw_punch();

	  // Stem holding.
	  mount_meat = 8;  // The 'meat' before we hit the butt-surface
	  mount_screw_len = dial_thick - dial_stem_pos + mount_meat;
	  translate([stem_mount_screw_distance/2, -mount_meat, max(m3_head_dia/2+1, stem_high/2)])
	       rotate([-90, 0, 0]) m3_screw(len=mount_screw_len,
					    nut_at=mount_meat+bottom_mount_back_offset-m3_nut_dia+m3_nut_thick);
	  translate([-stem_mount_screw_distance/2, -mount_meat, max(m3_head_dia/2+1, stem_high/2)])
	       rotate([-90, 0, 0]) m3_screw(len=mount_screw_len,
					    nut_at=mount_meat+bottom_mount_back_offset-m3_nut_dia+m3_nut_thick);
	  translate([0, dial_thick - dial_stem_pos, 0]) {
	       battery_box_punch();
	       if (cable_slots) battery_power_punch();
	  }
     }
}

// Separating behind and front of dial.
module dial_separator() {
     translate([-50, -100, -e]) cube([100, 100, stem_high+dial_wall+8]);
}

module dial_battery_lid() {
     intersection() {
	  dial_case();
	  translate([0, dial_thick - dial_stem_pos + fit_tolerance, 0]) battery_box_separator();
	  // Make lid a tiny bit shorter at the bottom, so that it fits
	  // comfortably on the screwed-down case.
	  translate([0, 0, 100+0.5]) cube([200, 200, 200], center=true);
     }
}

module dial_backend() {
     difference() {
	  dial_case();
	  dial_separator();
	  translate([0, dial_thick - dial_stem_pos, 0]) battery_box_separator();
     }
}

module dial_frontend() {
     intersection() {
	  dial_case();
	  translate([0, -fit_tolerance, -fit_tolerance]) dial_separator();
     }
}

module leg_plate() {
     difference() {
	  color("#f0f0ff", alpha=0.3) cylinder(r=leg_plate_radius, h=leg_plate_thick);
	  // Hole pattern
	  translate([0, 0, -1]) bottom_screw_punch();
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

if (true) {
     demo_leg_plate();
     color("yellow") render() dial_backend();
     color("red") render() dial_frontend();
     color("blue") render() dial_battery_lid();
}
