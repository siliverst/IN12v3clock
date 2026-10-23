#######################
# SDCC Makefile for building and flashing STM8 project
# Outputs are created in the './SDCC' directory.
#######################

# Compiler and Flags
CC               = sdcc
LD               = sdcc
OPTIMIZE         = --opt-code-size --max-allocs-per-node 100000
#OPTIMIZE         = --opt-code-speed
CFLAGS           = -mstm8 --std-sdcc99 --std-c99 $(OPTIMIZE)
LFLAGS           = -mstm8 -lstm8 --out-fmt-ihx

# Directories and Targets
OUTPUT_DIR       = SDCC
IHX_TARGET       = $(OUTPUT_DIR)/main.ihx
TARGET           = $(OUTPUT_DIR)/main.s19

PRJ_SRC_DIR      = .
PRJ_INC_DIR      = STM8_headers/include
PRJ_SOURCE       = $(foreach d, $(PRJ_SRC_DIR), $(wildcard $(d)/*.c))
PRJ_HEADER       = $(foreach d, $(PRJ_INC_DIR), $(wildcard $(d)/*.h))
PRJ_OBJECTS      = $(addprefix $(OUTPUT_DIR)/, $(notdir $(PRJ_SOURCE:.c=.rel)))

SOURCE           = $(PRJ_SOURCE)
HEADER           = $(PRJ_HEADER)
OBJECTS          = $(PRJ_OBJECTS)

INCLUDE          = $(foreach d, $(PRJ_INC_DIR), $(addprefix -I, $(d)))

# Makefile Search Paths
vpath %.c $(PRJ_SRC_DIR)
vpath %.h $(PRJ_INC_DIR)

# Rules
.PHONY: clean all default flash swim serial
.PRECIOUS: $(IHX_TARGET) $(TARGET) $(OBJECTS)

default: $(OUTPUT_DIR) $(TARGET)

all: default

# Create output folder
$(OUTPUT_DIR):
	@mkdir -p $(OUTPUT_DIR)

# Link target
$(IHX_TARGET): $(OBJECTS)
	@echo "Linking: $(OBJECTS) -> $(IHX_TARGET)"
	$(LD) $(LFLAGS) -o $@ $(OBJECTS)

# Convert IHX to S19
$(TARGET): $(IHX_TARGET)
	@echo "Converting: $(IHX_TARGET) -> $(TARGET)"
	objcopy -I ihex -O srec $< $@

# Compile objects
$(OBJECTS): $(SOURCE) $(HEADER)
$(OUTPUT_DIR)/%.rel: %.c
	@echo "Compiling: $< -> $@"
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

# Clean up build files
clean:
	@echo "Cleaning up build files..."
	@rm -rf $(OUTPUT_DIR)/*

# Upload via STM8 bootloader
serial:
	@echo "Uploading via STM8 bootloader: $(TARGET)"
	$(stm8gal_PATH) -port $(stm8gal_PORT) -write-file $(TARGET) -reset 0 -verify 0 -verbose 1
