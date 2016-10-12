PROJ=msf
CXX=avr-gcc
LIBS=-L /usr/local/avr/lib/avr5 -lm
#LIBS=-L ~/lib -lm
MCU=-mmcu=atmega328p
CPU_SPEED=-DF_CPU=16000000UL
CFLAGS=  $(MCU) $(CPU_SPEED) -Werror -Wall -Os -Wl,-Map=$(PROJ).map,--gc-sections -ffunction-sections -fdata-sections --std=c99

CC = avr-gcc
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
SIZE = avr-size
AR = avr-ar rcs
NM = avr-nm
PORT=/dev/cuaU0 # FreeBSD
ifeq ($(shell uname),Linux)
        PORT=/dev/ttyUSB0
        PORT=/dev/ttyACM0
endif
BAUD=57600
BAUD=115200

#INCLUDE=-I ./ -I /opt/avr/include
INCLUDE=-I ./ -I /usr/local/avr/include/

OBJS := main.o util.o lcd.o max7219.o keypad.o at24c32.o msf.o ds3231.o pcf8574.o twimaster.o spi.o led.o
#OBJS := main.o util.o lcd.o unixtime.o max7219.o keypad.o at24c32.o msf.o ds3231.o pcf8574.o twimaster.o spi.o led.o


default: build upload

upload:
	avrdude -V -F -p m328p -c arduino -b $(BAUD) -Uflash:w:$(PROJ).hex -P$(PORT)

build: $(PROJ).hex

$(PROJ).hex: $(PROJ).elf
	avr-objcopy -O ihex $< $@

connect:
	screen $(PORT)  $(BAUD)

# link
$(PROJ).elf: $(OBJS)
	$(CXX) -v $(CFLAGS) $(OBJS) $(LIBS) -o $@

# pull in dependency info for *existing* .o files
-include $(OBJS:.o=.d)

# compile and generate dependency info;
# more complicated dependency computation, so all prereqs listed
# will also become command-less, prereq-less targets
#   sed:    strip the target (everything before colon)
#   sed:    remove any continuation backslashes
#   fmt -1: list words one per line
#   sed:    strip leading spaces
#   sed:    add trailing colons
%.o: %.c
	$(CXX) -c $(CFLAGS) $(INCLUDE) $*.c -o $*.o
	$(CXX) -MM $(CFLAGS) $(INCLUDE) $*.c > $*.d
	@mv -f $*.d $*.d.tmp
	@sed -e 's|.*:|$*.o:|' < $*.d.tmp > $*.d
	@sed -e 's/.*://' -e 's/\\$$//' < $*.d.tmp | fmt -1 | \
	  sed -e 's/^ *//' -e 's/$$/:/' >> $*.d
	@rm -f $*.d.tmp

# remove compilation products
clean:
	rm -f $(PROJ) *.o *.d *.hex *.map *.elf
