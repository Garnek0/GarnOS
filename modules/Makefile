
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
override MOD_OBJ := $(CFILES:.c=.sys)
override MODULES = $(shell find -L . -type f -name '*.sys' | sed 's!.*/!!')

override INITRD_MODS := ide.sys ahci.sys
override INITRD_AUTOREG := initreg.txt
override AUTOREG := autoreg.txt

override HEADER_DEPS := $(CFILES:.c=.d)

.PHONY: all
all: $(MOD_OBJ) initrd autoreg sysroot_cpy

%.sys: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@
	cp $@ .
	rm $@

.PHONY: autoreg
autoreg:
	touch $(AUTOREG)
	echo "${MODULES}" | sed 's/ /\n/g' > $(AUTOREG)
	cp $(AUTOREG) ../sysroot

.PHONY: initrd
initrd:
	touch $(INITRD_AUTOREG)
	echo "${INITRD_MODS}" | sed 's/ /\n/g' > $(INITRD_AUTOREG)
	tar -cvf $(INITRD_FILENAME) $(INITRD_MODS) $(INITRD_AUTOREG)
	cp $(INITRD_FILENAME) ../sysroot/$(INITRD_FILENAME)

.PHONY: sysroot_cpy
sysroot_cpy:
	cp $(MODULES) ../sysroot

.PHONY: clean
clean:
	rm -rf $(HEADER_DEPS)
	rm -rf $(INITRD_AUTOREG) $(AUTOREG)
	rm -rf $(INITRD_FILENAME)
	rm -rf *.sys