
include ../.config

INITRD_FILENAME = initrd.grd

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
    -I../include \
    $(CPPFLAGS) \
    -MMD \
    -MP

CC := ../hosttools/bin/x86_64-pc-garn-mlibc-gcc
LD := ../hosttools/bin/x86_64-pc-garn-mlibc-ld

CFILES := $(shell find -L . -type f -name '*.c')
MOD_OBJ := $(CFILES:.c=.sys)
MODULES = $(shell find -L . -type f -name '*.sys' | sed 's!.*/!!')

INITRD_MODS := 

ifdef CONFIG_INCLUDE_IDE_DRIVER
INITRD_MODS += ide.sys
endif

ifdef CONFIG_INCLUDE_AHCI_DRIVER
INITRD_MODS += ahci.sys
endif

INITRD_AUTOREG := initreg.txt
AUTOREG := autoreg.txt

HEADER_DEPS := $(CFILES:.c=.d)

.PHONY: all
all: buildmods initrd drv_dir autoreg sysroot_cpy
	
.PHONY: buildmods
buildmods:
	@for a in $$(ls); do \
	    if [ -d $$a ]; then \
	        echo "Building module(s) in $$a/"; \
	        $(MAKE) -C $$a CFLAGS="$(CFLAGS)" CPPFLAGS="$(CPPFLAGS) -I../../include" CC="../$(CC)" LD="../$(LD)"; \
	    fi; \
	done;

.PHONY: autoreg
autoreg:
	touch $(AUTOREG)
	echo "${MODULES}" | sed 's/ /\n/g' | sed 's/^/\drv\//g' > $(AUTOREG)
	cp $(AUTOREG) ../sysroot/drv

.PHONY: initrd
initrd:
	touch $(INITRD_AUTOREG)
	echo "${INITRD_MODS}" | sed 's/ /\n/g' | sed 's/^/\drv\//g' > $(INITRD_AUTOREG)
	mkdir -p drv
	cp $(INITRD_MODS) $(INITRD_AUTOREG) drv
	tar -cvf $(INITRD_FILENAME) drv
	rm -rf drv
	cp $(INITRD_FILENAME) ../sysroot/$(INITRD_FILENAME)

.PHONY: sysroot_cpy
sysroot_cpy:
	cp $(MODULES) ../sysroot/drv

.PHONY: clean
clean:
	rm -rf $(HEADER_DEPS)
	rm -rf $(INITRD_AUTOREG) $(AUTOREG)
	rm -rf $(INITRD_FILENAME)
	rm -rf *.sys

drv_dir:
	mkdir -p ../sysroot/drv