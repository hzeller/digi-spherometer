TARGETS=fab/spherometer_frame_main_block.stl \
        fab/spherometer_frame_stem_squeeze_block.stl \
        fab/spherometer_frame_battery_lid.stl \
        fab/display_front_part.stl fab/display_transition_part.stl \
	fab/display_case_button.stl \
        fab/leg_plate_2d.dxf


GIT_HASH=$(shell git log --date=short --pretty=format:'%h' -n 1 frame.scad)
GIT_DATE=$(shell git log --date=short --pretty=format:'%cd' -n 1 frame.scad)

all: $(TARGETS)

%.stl: %.scad
	openscad -D'version_id="$(GIT_HASH)"' -D'version_date="$(GIT_DATE)"' \
                 -D'print_quality=true' -o $@ $^

%.dxf: %.scad
	openscad -D'print_quality=true' -o $@ $^

fab/%.scad : frame.scad
	mkdir -p fab
	echo "use <../frame.scad>; $*();" > $@

clean:
	rm -f $(TARGETS)
