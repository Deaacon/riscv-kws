ARCH    ?= esp32c3
PROG    ?= firmware

CC       = riscv32-unknown-elf-gcc
LD       = riscv32-unknown-elf-ld

CFLAGS   = -W -Wall -march=rv32imc -mabi=ilp32 \
           -fno-builtin -Os \
           -ffunction-sections -fdata-sections \
           -Iinc -Imodel -Idevice/$(ARCH)
LDFLAGS  = -T device/$(ARCH)/link.ld \
           -nostdlib -nostartfiles \
           -Wl,--gc-sections

ESPFLAGS = --chip $(ARCH)
ESPTOOL  = python -m esptool
ESPMON   = python -m esp_idf_monitor

SRCS = $(wildcard src/*.c) \
       $(wildcard device/*.c)
OBJS = $(SRCS:.c=.o)

all: $(PROG).bin

$(PROG).bin: $(PROG).elf
	$(ESPTOOL) $(ESPFLAGS) elf2image $(PROG).elf

$(PROG).elf: $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $(PROG).elf

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

flash: $(PROG).bin
	$(ESPTOOL) $(ESPFLAGS) write_flash --flash_mode dio 0x0 $(PROG).bin

monitor:
	$(ESPMON)

clean:
	rm -f $(OBJS) $(PROG).elf $(PROG).bin
