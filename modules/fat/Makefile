MODULE_NAME := fat

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
	-I../../ \
    -I../include \
    $(CPPFLAGS) \
    -MMD \
    -MP


CFILES := $(shell find -L . -type f -name '*.c')
OBJ := $(CFILES:.c=.c.o)

.PHONY: all
all:
	$(CC) $(CFLAGS) $(CPPFLAGS) $(CFILES) -o $(MODULE_NAME).sys -r
	cp $(MODULE_NAME).sys ..
	rm $(MODULE_NAME).sys
