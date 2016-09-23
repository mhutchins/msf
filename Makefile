CXX=avr-gcc
INCLUDE=-I ../arduino-lib/ -I ./

LIBS=-L ~/lib -lm -larduino
MCU=-mmcu=atmega328p
CPU_SPEED=-DF_CPU=16000000UL
CFLAGS=$(MCU) $(CPU_SPEED) -Os -w -Wl,--gc-sections -ffunction-sections -fdata-sections
PORT=/dev/cuaU0 # FreeBSD
ifeq ($(shell uname),Linux)
	PORT=/dev/ttyACM0
endif

default: build upload

build: msf.hex

msf.hex: msf.elf
	avr-objcopy -O ihex $< $@

OBJECTS= timer.c ds3231.c pcf8574.c twimaster.c spi.c # Put other objects here
msf.elf: msf.c $(OBJECTS)
	$(CXX) $(CFLAGS) $(INCLUDE) $^ -o $@ $(LIBS)

upload:
	avrdude -V -F -p m328p -c arduino -b 115200 -Uflash:w:msf.hex -P$(PORT)

clean:
	@echo -n Cleaning ...
	$(shell rm msf.elf 2> /dev/null)
	$(shell rm msf.hex 2> /dev/null)
	$(shell rm *.o 2> /dev/null)
	@echo " done"

%.o: %.c
	$(CXX) $< $(CFLAGS) $(INCLUDE) -c -o $@
