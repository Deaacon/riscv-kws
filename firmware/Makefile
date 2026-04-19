PROG  = firmware
CC       = riscv32-unknown-elf-gcc
LD       = riscv32-unknown-elf-ld
CFLAGS   = -W -Wall -march=rv32imc -mabi=ilp32 \
           -fno-builtin -Os \
           -ffunction-sections -fdata-sections \
           -I.
LDFLAGS  = -T link.ld -nostdlib -nostartfiles \
           -Wl,--gc-sections
ESPFLAGS = --chip esp32c3
ESPTOOL  = python -m esptool
ESPMON   = python -m esp_idf_monitor

SRCS = main.c startup.c
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
	rm -f *.o *.elf *.bin
