ARCH    ?= esp32c3
PROG    ?= firmware
OUT_DIR  = .output

CC       = riscv32-unknown-elf-gcc
LD       = riscv32-unknown-elf-ld

TFLM_DIR = lib/tflm

ARCHFLAGS = -march=rv32imc -mabi=ilp32
CFLAGS   = -W -Wall $(ARCHFLAGS) -g \
           -fno-builtin -Os \
           -ffunction-sections -fdata-sections \
           -Iinc -Imodel -Idevice/$(ARCH) -Idevice \
           -isystem device/$(ARCH) -isystem device \
           -DFIXED_POINT=16 \
           -I$(TFLM_DIR) \
           -I$(TFLM_DIR)/third_party/kissfft \
           -I$(TFLM_DIR)/third_party/kissfft/tools
LDFLAGS  = -T device/$(ARCH)/link.ld $(ARCHFLAGS) -g \
           -L./device/$(ARCH)/ld \
           -nostdlib -nostartfiles \
           -Wl,--gc-sections

ESPFLAGS = --chip $(ARCH)
ESPTOOL  = python -m esptool
ESPMON   = python -m esp_idf_monitor

SRCS     = $(wildcard src/*.c) \
           $(wildcard device/*.c) \
           $(wildcard lib/tflm/third_party/kissfft/tools/*.c) \
           $(wildcard lib/tflm/third_party/kissfft/*.c) \
           $(wildcard device/$(ARCH)/*.c)
OBJS     = $(patsubst %.c, $(OUT_DIR)/%.o, $(SRCS))

BIN      = $(OUT_DIR)/$(PROG).bin
ELF      = $(OUT_DIR)/$(PROG).elf

all: $(BIN)

$(BIN): $(ELF)
	$(ESPTOOL) $(ESPFLAGS) elf2image $(ELF)

$(ELF): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $(ELF) -lm -lgcc

$(OUT_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

flash: $(BIN)
	$(ESPTOOL) $(ESPFLAGS) write-flash --flash-mode dio 0x0 $(BIN)

monitor:
	$(ESPMON)

clean:
	rm -rf $(OUT_DIR)
