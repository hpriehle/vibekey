# Generate STL files from OpenSCAD source
# Requires: OpenSCAD (https://openscad.org)
#   brew install openscad    # macOS
#   apt install openscad     # Ubuntu/Debian
#   choco install openscad   # Windows

SCAD = one_key_case.scad
OUT_DIR = stl

all: $(OUT_DIR)/vibekey-base.stl $(OUT_DIR)/vibekey-top.stl

$(OUT_DIR)/vibekey-base.stl: $(SCAD)
	@mkdir -p $(OUT_DIR)
	openscad -o $@ -D 'render_part="base"' $(SCAD) 2>/dev/null || \
	openscad -o $@ $(SCAD)

$(OUT_DIR)/vibekey-top.stl: $(SCAD)
	@mkdir -p $(OUT_DIR)
	openscad -o $@ -D 'render_part="top"' $(SCAD) 2>/dev/null || \
	openscad -o $@ $(SCAD)

clean:
	rm -rf $(OUT_DIR)

.PHONY: all clean
