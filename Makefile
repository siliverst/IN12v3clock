#######################
# SDCC build on Linux / macOS: make, make clean, make flash
# The same options as tools/build.py sdcc, output in build/sdcc (precompiled SDCC/ is not touched).
# Requires: sdcc (4.5 is tested, older versions may not fit into 8 KB), make, objcopy (binutils);
# make flash: stm8flash (https://github.com/vdudouyt/stm8flash) and ST-LINK V2.
#######################

CC               = sdcc
LD               = sdcc
OPTIMIZE         = --opt-code-size --max-allocs-per-node 100000
CFLAGS           = -mstm8 --std-sdcc99 $(OPTIMIZE)
LFLAGS           = -mstm8 -lstm8 --out-fmt-ihx

OUTPUT_DIR       = build/sdcc
IHX_TARGET       = $(OUTPUT_DIR)/main.ihx
TARGET           = $(OUTPUT_DIR)/main.s19

PRJ_SRC_DIR      = .
PRJ_INC_DIR      = STM8_headers/include
SOURCE           = $(wildcard $(PRJ_SRC_DIR)/*.c)
HEADER           = $(wildcard $(PRJ_SRC_DIR)/*.h)
OBJECTS          = $(addprefix $(OUTPUT_DIR)/, $(notdir $(SOURCE:.c=.rel)))
INCLUDE          = -I$(PRJ_INC_DIR)

vpath %.c $(PRJ_SRC_DIR)

.PHONY: all clean flash
.PRECIOUS: $(IHX_TARGET) $(TARGET) $(OBJECTS)

all: $(TARGET)

$(OUTPUT_DIR):
	@mkdir -p $(OUTPUT_DIR)

$(IHX_TARGET): $(OBJECTS)
	$(LD) $(LFLAGS) -o $@ $(OBJECTS)

$(TARGET): $(IHX_TARGET)
	objcopy -I ihex -O srec $< $@
	@grep -m1 "^CODE" $(OUTPUT_DIR)/main.map || true

# every object depends on all headers (simple and safe for a small project)
$(OUTPUT_DIR)/%.rel: %.c $(HEADER) | $(OUTPUT_DIR)
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

clean:
	rm -rf $(OUTPUT_DIR)

# flash only, the option bytes and EEPROM are not touched (set AFR0 once, see README)
flash: $(IHX_TARGET)
	stm8flash -c stlinkv2 -p stm8s003f3 -s flash -w $(IHX_TARGET)
