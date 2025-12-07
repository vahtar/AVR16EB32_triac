# AVR16EB32 Triac Control Makefile

# Project name
PROJECT = triac_control

# MCU settings
MCU = avr16eb32
F_CPU = 20000000UL

# Compiler and tools
CC = avr-gcc
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
SIZE = avr-size
AVRDUDE = avrdude

# Directories
SRC_DIR = src
INC_DIR = include
BUILD_DIR = build

# Source files
SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC))

# Compiler flags
CFLAGS = -mmcu=$(MCU) -DF_CPU=$(F_CPU) -I$(INC_DIR)
CFLAGS += -Os -Wall -Wextra -std=gnu99
CFLAGS += -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
CFLAGS += -ffunction-sections -fdata-sections

# Linker flags
LDFLAGS = -mmcu=$(MCU)
LDFLAGS += -Wl,--gc-sections
LDFLAGS += -Wl,-Map=$(BUILD_DIR)/$(PROJECT).map

# Output formats
HEX = $(BUILD_DIR)/$(PROJECT).hex
ELF = $(BUILD_DIR)/$(PROJECT).elf
LSS = $(BUILD_DIR)/$(PROJECT).lss

# Default target
all: $(BUILD_DIR) $(HEX) $(LSS) size

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compile source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Link object files
$(ELF): $(OBJ)
	$(CC) $(LDFLAGS) $(OBJ) -o $@

# Create hex file
$(HEX): $(ELF)
	$(OBJCOPY) -O ihex -R .eeprom -R .fuse -R .lock -R .signature $< $@

# Create extended listing
$(LSS): $(ELF)
	$(OBJDUMP) -h -S $< > $@

# Show size information
size: $(ELF)
	@echo
	@echo "Size information:"
	@$(SIZE) -C --mcu=$(MCU) $<

# Clean build files
clean:
	rm -rf $(BUILD_DIR)

# Program the device (requires avrdude configuration)
flash: $(HEX)
	$(AVRDUDE) -p $(MCU) -c usbasp -U flash:w:$(HEX):i

# Phony targets
.PHONY: all clean flash size

# Dependencies
-include $(OBJ:.o=.d)

# Generate dependencies
$(BUILD_DIR)/%.d: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	@$(CC) $(CFLAGS) -MM -MT $(BUILD_DIR)/$*.o $< > $@
