CFLAGS := \
    -Wall \
    -Wextra \
    -fno-lto \
	-fno-PIE \
    -mabi=sysv \
    -mno-80387 \
    -mno-mmx \
    -mno-sse \
    -mno-sse2

CC = ../hosttools/bin/x86_64-pc-garn-mlibc-gcc
LD = ../hosttools/bin/x86_64-pc-garn-mlibc-ld

CFILES := $(shell find -L . -type f -name '*.c')
EXEC := $(CFILES:.c=.elf)

HEADER_DEPS := $(CFILES:.c=.d)

all: bin_dir $(EXEC)

bin_dir:
	mkdir -p ../sysroot/bin

%.elf: %.c
	$(CC) $(CFLAGS) $< -o $@
	cp $@ ../sysroot/bin/$@

.PHONY: clean
clean:
	rm -f $(EXEC)