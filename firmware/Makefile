CC = avr-gcc -mmcu=atmega88 -Wall 
CCFLAGS = -O3 -fdata-sections -ffunction-sections -DNDEBUG -std=c99 -I/usr/local/avr/include/ 
LDFLAGS = -lm -L/usr/local/avr/lib/avr4/  -Wl,--gc-sections
AVRDUDE = avrdude -p m88 -c usbasp -e

APP = usynth

%.o: %.c *.h
	$(CC) $(CCFLAGS) -o $@ $< -c 
	
all:
	make $(APP)
	
$(APP): main.o uart.o usynth.o control.o
	$(CC) $^ -o $@.elf $(LDFLAGS)
	avr-objcopy -j .text -j .data -O ihex $(APP).elf $(APP).hex

gui: 
	gcc -o control gtk-control.c `pkg-config --libs --cflags gtk+-2.0`
	
install:
	$(AVRDUDE) -U flash:w:$(APP).hex 


# Fuse high byte:
# 0xc9 = 1 1 0 1   1 1 1 1 <-- BOOTRST (boot reset vector at 0x0000)
#        ^ ^ ^ ^   ^ ^ ^------ BOOTSZ0
#        | | | |   | +-------- BOOTSZ1
#        | | | |   + --------- EESAVE (don't preserve EEPROM over chip erase)
#        | | | +-------------- CKOPT (full output swing)
#        | | +---------------- SPIEN (allow serial programming)
#        | +------------------ WDTON (WDT not always on)
#        +-------------------- RSTDISBL (reset pin is enabled)
# Fuse low byte:
# 0x9f = 1 1 1 0   0 1 1 1
#        ^ ^ \ /   \--+--/
#        | |  |       +------- CKSEL 3..0 (external >8M crystal)
#        | |  +--------------- SUT 1..0 (crystal osc, BOD enabled)
#        | +------------------ CKOUT 
#        +-------------------- DIV8 
fuse:
	$(AVRDUDE) -U hfuse:w:0xdf:m -U lfuse:w:0xe7:m 
	#$(AVRDUDE) -U hfuse:w:0xc9:m -U lfuse:w:0x91:m
clean:
	rm -rf *.o *.elf *.hex
