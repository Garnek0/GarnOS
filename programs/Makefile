override CFLAGS += \
    -Wall \
    -Wextra \
    -fno-lto \
	-fno-PIE \
    -mabi=sysv \
    -mno-80387 \
    -mno-mmx \
    -mno-sse \
    -mno-sse2

override CC = ../hosttools/bin/x86_64-pc-garn-mlibc-gcc
override LD = ../hosttools/bin/x86_64-pc-garn-mlibc-ld

override CFILES := $(shell find -L . -type f -name '*.c')
override EXEC := $(CFILES:.c=.elf)

override HEADER_DEPS := $(CFILES:.c=.d)

all: $(EXEC)

%.elf: %.c
	$(CC) $(CFLAGS) $< -o $@
	cp $@ ../sysroot/bin/$@

.PHONY: clean
clean:
	rm -f $(EXEC)