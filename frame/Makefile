TARGETS=frame-generated-dial_frontend.stl frame-generated-dial_backend.stl \
        frame-generated-dial_battery_lid.stl frame-generated-display_part.stl \
	frame-generated-leg_plate_2d.dxf

GIT_HASH=$(shell git log --date=short --pretty=format:'%h' -n 1 frame.scad)
GIT_DATE=$(shell git log --date=short --pretty=format:'%cd' -n 1 frame.scad)

all: $(TARGETS)

%.stl: %.scad
	openscad -D'version_id="$(GIT_HASH)"' -D'version_date="$(GIT_DATE)"' \
                 -D'print_quality=true' -o $@ $^

%.dxf: %.scad
	openscad -o $@ $^

frame-generated-%.scad : frame.scad
	echo "use <frame.scad>; $*();" > $@

clean:
	rm -f $(TARGETS)
