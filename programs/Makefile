override CFLAGS += \
    -Wall \
    -Wextra \
    -std=gnu11 \
    -ffreestanding \
    -fno-stack-protector \
    -fno-stack-check \
    -fno-lto \
	-fno-PIE \
	-mcmodel=large \
    -m64 \
    -march=x86-64 \
    -mabi=sysv \
    -mno-80387 \
    -mno-mmx \
    -mno-sse \
    -mno-sse2 \
    -mno-red-zone \
	-nostdlib \
	-static \
	-march=x86-64

override LDFLAGS += \
    -m elf_x86_64 \
    -nostdlib \
    -static \
    -pie \
    --no-dynamic-linker \
    -z text \
    -z max-page-size=0x1000 \

override CC = gcc
override LD = ld

override CFILES := $(shell find -L . -type f -name '*.c')
override OBJ := $(CFILES:.c=.elf)

override HEADER_DEPS := $(CFILES:.c=.d)

all: $(OBJ)

%.elf: %.c
	$(CC) $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	rm $(OBJ)