TARGETS=frame-generated-dial_frontend.stl frame-generated-dial_backend.stl \
        frame-generated-dial_battery_lid.stl \
	frame-generated-leg_plate_2d.dxf

all: $(TARGETS)

%.stl: %.scad
	openscad -o $@ $^

%.dxf: %.scad
	openscad -o $@ $^

frame-generated-%.scad : frame.scad
	echo "use <frame.scad>; $*();" > $@

clean:
	rm -f $(TARGETS)
