ARCH    ?= esp32c3
PROG    ?= firmware
OUT_DIR  = .output

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

SRCS     = $(wildcard src/*.c) \
           $(wildcard device/*.c)
OBJS     = $(patsubst %.c, $(OUT_DIR)/%.o, $(SRCS))

BIN      = $(OUT_DIR)/$(PROG).bin
ELF      = $(OUT_DIR)/$(PROG).elf

all: $(BIN)

$(BIN): $(ELF)
	$(ESPTOOL) $(ESPFLAGS) elf2image $(ELF)

$(ELF): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $(ELF)

$(OUT_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

flash: $(BIN)
	$(ESPTOOL) $(ESPFLAGS) write_flash --flash_mode dio 0x0 $(BIN)

monitor:
	$(ESPMON)

clean:
	rm -rf $(OUT_DIR)
