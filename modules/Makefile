
INITRD_FILENAME = initrd.grd

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
    -mno-red-zone

override CPPFLAGS := \
    -I. \
    -I../kernel \
    -I../kernel/klibc/include \
    $(CPPFLAGS) \
    -MMD \
    -MP

override CC = gcc

override CFILES := $(shell find -L . -type f -name '*.c')
override MOD_OBJ := $(CFILES:.c=.mod)
override INITRD_MODS := ide.mod ahci.mod
override HEADER_DEPS := $(CFILES:.c=.d)

.PHONY: all
all: $(MOD_OBJ) initrd

%.mod: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@
	cp $@ .
	rm $@

.PHONY: initrd
initrd:
	tar -cvf $(INITRD_FILENAME) $(INITRD_MODS)

.PHONY: clean
clean:
	rm -rf $(HEADER_DEPS)
	rm -rf initrd.grd
	rm -rf *.mod