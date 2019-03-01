// Mounting frame holding the indicator and display
// CAVE: Super-hacky right now while exploring different shape.
$fn=128;
e=0.01;

inset_m3_dia=3.8;
inset_m3_len=4;
m3_head_dia=6;

dial_dia=57.5;
dial_wall=2;
dial_thick=25.2;
dial_stem_pos = 21.4-4;  // Position of stem from the front.
dial_cable_pos=12;   // Position of the cable channel from the front

stem_dia=8;
stem_high=21.5 - 6;   // height stem - acrylic thick
stem_mount_screw_distance=stem_dia + 8;

aa_dia=15;
aa_len=50.5 + 8;  // for contacts
aa_wall=2;
aa_dist = 8;      // Distance between batteries

base_dia=dial_stem_pos*2;

display_wide=55;
display_high=35;
display_front_radius=5;

fit_tolerance=0.3;  // Tolerance of parts in contact.

module inset_with_screw(distance=3) {
     translate([0, 0, -inset_m3_len+e]) cylinder(r=inset_m3_dia/2, h=inset_m3_len);
     cylinder(r=3.2/2, h=distance+e);
     translate([0, 0, distance]) cylinder(r=m3_head_dia/2, h=20);
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
     base_high=2;
     translate([0, -base_dia/2, 0]) hull() { // Move to the front.
	  translate([-display_wide/2+display_front_radius,
		     -display_high+display_front_radius, 0])
	       cylinder(r=display_front_radius, h=base_high);
	  translate([+display_wide/2-display_front_radius,
		     -display_high+display_front_radius, 0])
	       cylinder(r=display_front_radius, h=base_high);
	  translate([-display_wide/2, 0, 0]) cube([display_wide, e, base_high]);
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
     rotate([90, 0, 0])
	  translate([0, wall_r+stem_high, -dial_thick+dial_stem_pos])
	  difference() {
	  cylinder(r=wall_r, h=dial_thick);
	  translate([-50, 20, -e]) cube([100, 100, 100]);
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
	  if (punch) {
	       translate([-aa_dia/2, 0, 0]) cube([aa_dia, 2*aa_dia, aa_len]);
	       rotate([0, 0, -10]) translate([0, -aa_dia/2, aa_len/2]) rotate([90, 0, 0]) aa_punch();
	  }
     }
}

module battery_box_punch() {
     height=aa_len + 2*aa_wall;
     width=2*aa_dia + aa_dist + 2*aa_wall;
     thick=aa_dia+2*aa_wall;

     translate([0, thick/2, 0]) {
	  translate([-(aa_dia+aa_dist)/2, 0, aa_wall+aa_len/2]) aabat(punch=true);
	  translate([+(aa_dia+aa_dist)/2, 0, aa_wall+aa_len/2]) rotate([0, 180, 0]) aabat(punch=true);

	  empty_space=0.1;
	  translate([-(aa_dia+aa_dist)/2, -aa_dia/2, aa_wall])
	       cube([aa_dia+aa_dist, 40, empty_space*height]);
	  translate([-(aa_dia+aa_dist)/2, -aa_dia/2, height-aa_wall-empty_space*height])
	       cube([aa_dia+aa_dist, 40, empty_space*height]);

	  translate([0, -aa_dia/2+aa_wall, height/2]) rotate([-90, 0, 0]) cylinder(r=3.5/2, h=40);
	  translate([0, aa_dia/2-inset_m3_len, height/2]) rotate([-90, 0, 0]) cylinder(r=inset_m3_dia/2, h=40);
	  //translate([-6, 0.2*-aa_dia/2, aa_wall]) cube([12, 30, aa_len]);
     }
}

module battery_box() {
     height=aa_len + 2*aa_wall;
     width=2*aa_dia + aa_dist + 2*aa_wall;
     thick=aa_dia+aa_wall;
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
	  translate([stem_mount_screw_distance/2, 0, stem_high/2+4])
	       rotate([90, 0, 0]) inset_with_screw();
	  translate([-stem_mount_screw_distance/2, 0, stem_high/2+4])
	       rotate([90, 0, 0]) inset_with_screw();
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

module dial_backend() {
     difference() {
	  dial_case();
	  dial_separator();
     }
}

module dial_frontend() {
     intersection() {
	  dial_case();
	  translate([0, -fit_tolerance, -fit_tolerance]) dial_separator();
     }
}

color("blue") render() dial_backend();
color("red") render() dial_frontend();
