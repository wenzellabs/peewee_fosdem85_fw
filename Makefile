# Name: Makefile
# Author: Urban Bieri
#         matze wenzel

# You should at least check the settings for
# DEVICE ....... The AVR device you compile for
# CLOCK ........ Target AVR clock rate in Hertz
# OBJECTS ...... The object files created from your source files. This list is
#                usually the same as the list of source files with suffix ".o".
# PROGRAMMER ... Options to avrdude which define the hardware you use for
#                uploading to the AVR and the interface where this hardware
#                is connected.

# we target the FOSDEM-85 with the libusb based micronucleus bootloader
# (without serial or CDC device)

PROJECT=peewee

DEVICE     = attiny85
CLOCK      = 16500000
PROGRAMMER = -c micronucleus
OBJECTS    = $(PROJECT).o

AVRDUDE = avrdude $(PROGRAMMER) -p $(DEVICE)
COMPILE = avr-gcc -Wall -Os -DF_CPU=$(CLOCK) -mmcu=$(DEVICE)

# symbolic targets:
all: $(PROJECT).hex

.c.o:
	$(COMPILE) -c $< -o $@

.S.o:
	$(COMPILE) -x assembler-with-cpp -c $< -o $@
# "-x assembler-with-cpp" should not be necessary since this is the default
# file type for the .S (with capital S) extension. However, upper case
# characters are not always preserved on Windows. To ensure WinAVR
# compatibility define the file type manually.

.c.s:
	$(COMPILE) -S $< -o $@

flash: all
	$(AVRDUDE) -U flash:w:$(PROJECT).hex:i --noverify-memory

clean:
	rm -f $(PROJECT).hex $(PROJECT).elf $(OBJECTS)

# file targets:
$(PROJECT).elf: $(OBJECTS)
	$(COMPILE) -o $(PROJECT).elf $(OBJECTS)

$(PROJECT).hex: $(PROJECT).elf
	rm -f $(PROJECT).hex
	avr-objcopy -j .text -j .data -O ihex $(PROJECT).elf $(PROJECT).hex
# If you have an EEPROM section, you must also create a hex file for the
# EEPROM and add it to the "flash" target.

# Targets for code debugging and analysis:
disasm:	$(PROJECT).elf
	avr-objdump -d $(PROJECT).elf

cpp:
	$(COMPILE) -E $(PROJECT).c
