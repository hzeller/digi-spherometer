TARGETS=fab/spherometer_frame_main_block.stl \
        fab/spherometer_frame_stem_squeeze_block.stl \
        fab/spherometer_frame_battery_lid.stl \
        fab/display_front_part.stl fab/display_transition_part.stl \
	fab/display_case_button.stl \
        fab/leg_plate_2d.dxf

TARGETS_MITUOYO=fab/mit_back_part.stl \
        fab/mit_front_display_part.stl \
        fab/mit_front_block_part.stl \
        fab/mit_battery_cover.stl \
        fab/spherometer_frame_battery_lid.stl \
	fab/display_case_button.stl


GIT_HASH=$(shell git log --date=short --pretty=format:'%h' -n 1 frame.scad)
GIT_DATE=$(shell git log --date=short --pretty=format:'%cd' -n 1 frame.scad)

OPENSCAD=openscad -q --backend Manifold  -D'$$fs=0.15' -D'$$fa=1'

mitutoyo: $(TARGETS_MITUOYO)

all: $(TARGETS)

%.stl: %.scad
	$(OPENSCAD) -D'version_id="$(GIT_HASH)"' \
                    -D'version_date="$(GIT_DATE)"' \
                    -D'print_quality=true' -o $@ $^

%.dxf: %.scad
	$(OPENSCAD) -D'print_quality=true' -o $@ $^

fab/%.scad : frame.scad
	@mkdir -p fab
	echo "use <../frame.scad>; $*();" > $@

clean:
	rm -f $(TARGETS) $(TARGETS_MITUOYO)
