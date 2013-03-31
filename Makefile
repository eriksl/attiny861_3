MCU			=		attiny861
MCUSPEED	=		16500000
PROGRAMMER	=		dragon_isp
PRGFLAGS	=		-b 0 -P usb

PROGRAM		=		main
OBJFILES	=		usbdrv/usbdrv.o usbdrv/usbdrvasm.o usbdrv/oddebug.o main.o
HEADERS		=		usbconfig.h usbdrv/oddebug.h usbdrv/usbdrv.h usbdrv/usbportability.h
HEXFILE		=		$(PROGRAM).hex
ELFFILE		=		$(PROGRAM).elf
PROGRAMMED	=		.programmed
CFLAGS		=		-Iusbdrv -I. -Wall -Winline -O3 -mmcu=$(MCU) -DF_CPU=$(MCUSPEED)UL \
					-fpack-struct -funroll-loops -funit-at-a-time -fno-keep-static-consts \
					-frename-registers
LDFLAGS		=		-Wall -mmcu=$(MCU)

.PHONY:				all clean hex
.SUFFIXES:
.SUFFIXES:			.c .o .elf .hex
.PRECIOUS:			.c .h

all:				$(PROGRAMMED)
hex:				$(HEXFILE)

$(PROGRAM).o:		$(PROGRAM).c $(HEADERS)

%.o:				%.S
					@echo "AS $< -> $@"
					@avr-gcc -x assembler-with-cpp -c $(CFLAGS) $< -o $@

%.o:				%.c
					@echo "CC $< -> $@"
					@avr-gcc -c $(CFLAGS) $< -o $@

%.s:				%.c
					@echo "CC (ASM) $< -> $@"
					@avr-gcc -S $(CFLAGS) $< -o $@

$(ELFFILE):			$(OBJFILES)
					@echo "LD $(OBJFILES) -> $@"
					@avr-gcc $(LDFLAGS) $(OBJFILES) -o $@

$(HEXFILE):			$(ELFFILE)
					@echo "OBJCOPY $< -> $@"
					@avr-objcopy -j .text -j .data -O ihex $< $@
					@sh -c 'avr-size $< | (read header; read text data bss junk; echo "SIZE: flash: $$[text + data] ram: $$[data + bss]")'

$(PROGRAMMED):		$(HEXFILE)
					@echo "AVRDUDE $^"
					@sh -c "avrdude -vv -c $(PROGRAMMER) -p $(MCU) $(PRGFLAGS) -U flash:w:$^ > $(PROGRAMMED) 2>&1"

clean:			
					@echo "RM $(OBJFILES) $(ELFFILE) $(HEXFILE) $(PROGRAMMED)"
					@-rm $(OBJFILES) $(ELFFILE) $(HEXFILE) 2> /dev/null || true
