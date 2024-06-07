MODULE_NAME := ide

CFLAGS := \
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
	-O2

CPPFLAGS := \
    -I. \
    -I../../include \
    $(CPPFLAGS) \
    -MMD \
    -MP

CC := ../../hosttools/bin/x86_64-pc-garn-mlibc-gcc
LD := ../../hosttools/bin/x86_64-pc-garn-mlibc-ld

override CFILES := $(shell find -L . -type f -name '*.c')
override OBJ := $(CFILES:.c=.c.o)

.PHONY: all
all:
	$(CC) $(CFLAGS) $(CPPFLAGS) $(CFILES) -o $(MODULE_NAME).sys -r
	cp $(MODULE_NAME).sys ..
	rm $(MODULE_NAME).sys