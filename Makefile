# Nuke built-in rules and variables.
override MAKEFLAGS += -rR

override IMAGE_NAME := garnos
override MODULES = $(shell find ./modules -name '*.sys')
override PROGRAMS = $(shell find ./programs -name '*.elf')
override AUTOREG = ./modules/autoreg.txt

# Convenience macro to reliably declare user overridable variables.
define DEFAULT_VAR =
    ifeq ($(origin $1),default)
        override $(1) := $(2)
    endif
    ifeq ($(origin $1),undefined)
        override $(1) := $(2)
    endif
endef

# Compiler for building the 'limine' executable for the host.
override DEFAULT_HOST_CC := cc
$(eval $(call DEFAULT_VAR,HOST_CC,$(DEFAULT_HOST_CC)))

.PHONY: all
all: $(IMAGE_NAME).iso

.PHONY: all-hdd
all-hdd: $(IMAGE_NAME).hdd

.PHONY: run
run: $(IMAGE_NAME).iso
	qemu-system-x86_64 -M q35 -smp 4 -m 2G -cdrom $(IMAGE_NAME).iso -boot d
	
.PHONY: run-uefi
run-uefi: ovmf $(IMAGE_NAME).iso
	qemu-system-x86_64 -M q35 -smp 4 -enable-kvm -m 2G -bios ovmf/OVMF.fd -cdrom $(IMAGE_NAME).iso -boot d

.PHONY: run-hdd
run-hdd: $(IMAGE_NAME).hdd
	qemu-system-x86_64 -M q35 -smp 4 -enable-kvm -debugcon stdio -m 2G -vga std -hda $(IMAGE_NAME).hdd

.PHONY: run-hdd-uefi
run-hdd-uefi: ovmf $(IMAGE_NAME).hdd
	qemu-system-x86_64 -M q35 -smp 4 -debugcon stdio -m 2G -enable-kvm -cpu max -vga std -bios ovmf/OVMF.fd -hda $(IMAGE_NAME).hdd

ovmf:
	mkdir -p ovmf
	cd ovmf && curl -o OVMF.fd https://retrage.github.io/edk2-nightly/bin/RELEASEX64_OVMF.fd

limine:
	git clone https://github.com/limine-bootloader/limine.git --branch=v7.x-binary --depth=1
	$(MAKE) -C limine CC="$(HOST_CC)"
	cp -v limine.cfg limine/limine-bios.sys limine/limine-bios-cd.bin limine/limine-uefi-cd.bin sysroot
	mkdir -p sysroot/efi/boot
	cp -v limine/BOOTX64.EFI sysroot/efi/boot/BOOTX64.EFI
	
.PHONY: kernel
kernel:
ifdef DEBUG
	$(MAKE) -C kernel DEBUG=y
else
	$(MAKE) -C kernel
endif

.PHONY: modules
modules:
ifdef DEBUG
	$(MAKE) -C modules DEBUG=y
else
	$(MAKE) -C modules
endif

.PHONY: programs
programs:
ifdef DEBUG
	$(MAKE) -C programs DEBUG=y
else
	$(MAKE) -C programs
endif

sysroot:
	rm -rf sysroot

	mkdir -p sysroot
	mkdir sysroot/efi
	mkdir sysroot/efi/boot

	cp -f release sysroot/release

.PHONY: all-toolchain
all-toolchain: toolchain libc

.PHONY: libc-headers
libc-headers: export DESTDIR=../../hosttools
libc-headers: 
	rm -rf mlibc
	git clone -b release-4.0 https://github.com/Garnek0/mlibc-garn mlibc
	cd mlibc; meson setup build -Dheaders_only=true --cross-file ci/garn-x86_64.cross-file --prefix / && ninja -C build install

.PHONY: libc
libc: export DESTDIR=../../hosttools
libc:
	export PATH=$(shell pwd)/hosttools/bin/:$$PATH; cd mlibc; rm -rf build; meson setup build -Ddefault_library=static -Dmlibc_no_headers=true --cross-file ci/garn-x86_64.cross-file --prefix / && ninja -C build install
	mv hosttools/lib/crt0.o hosttools/lib/gcc/x86_64-pc-garn-mlibc/13.2.0/
	mv hosttools/lib/crti.o hosttools/lib/gcc/x86_64-pc-garn-mlibc/13.2.0/
	mv hosttools/lib/crtn.o hosttools/lib/gcc/x86_64-pc-garn-mlibc/13.2.0/
	mv hosttools/lib/libc.a hosttools/lib/gcc/x86_64-pc-garn-mlibc/13.2.0/
	mv hosttools/lib/libcrypt.a hosttools/lib/gcc/x86_64-pc-garn-mlibc/13.2.0/
	mv hosttools/lib/libdl.a hosttools/lib/gcc/x86_64-pc-garn-mlibc/13.2.0/
	mv hosttools/lib/libm.a hosttools/lib/gcc/x86_64-pc-garn-mlibc/13.2.0/
	mv hosttools/lib/libpthread.a hosttools/lib/gcc/x86_64-pc-garn-mlibc/13.2.0/
	mv hosttools/lib/libresolv.a hosttools/lib/gcc/x86_64-pc-garn-mlibc/13.2.0/
	mv hosttools/lib/librt.a hosttools/lib/gcc/x86_64-pc-garn-mlibc/13.2.0/
	mv hosttools/lib/libutil.a hosttools/lib/gcc/x86_64-pc-garn-mlibc/13.2.0/

# I know this is not a very futureproof/ethical way of
# doing this :). I may use an actual fetch/patch/compile tool
# In the future such as jinx
.PHONY: toolchain
toolchain: export DESTDIR=
toolchain: libc-headers
	rm -rf toolchain
	mkdir toolchain
	git clone https://github.com/Garnek0/gcc-garn toolchain/gcc
	git clone https://github.com/Garnek0/binutils-gdb-garn toolchain/binutils
	cd toolchain && mkdir -p binutils-build && mkdir -p ../hosttools
	cd toolchain/binutils-build && ../binutils/configure --disable-shared --target=x86_64-pc-garn-mlibc --prefix=$(shell pwd)/hosttools --with-build-sysroot=$(shell pwd)/hosttools --disable-nls --disable-werror && make && make install
	cd toolchain && mkdir -p gcc-build
	cd toolchain/gcc-build && ../gcc/configure --disable-shared --target=x86_64-pc-garn-mlibc --with-headers=$(shell pwd)/hosttools/include --prefix=$(shell pwd)/hosttools --disable-nls --enable-languages=c,c++ --with-build-sysroot=$(shell pwd)/hosttools --enable-initfini-array --disable-shared && make all-gcc && make all-target-libgcc && make install-gcc && make install-target-libgcc	

$(IMAGE_NAME).iso: sysroot limine kernel modules programs
	rm -rf iso_root
	mkdir -p iso_root
	cp -vr sysroot/* iso_root/
	xorriso -as mkisofs -b limine-bios-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot limine-uefi-cd.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		iso_root -o $(IMAGE_NAME).iso
	./limine/limine bios-install $(IMAGE_NAME).iso
	rm -rf iso_root

$(IMAGE_NAME).hdd: sysroot limine kernel modules programs
	rm -f $(IMAGE_NAME).hdd
	dd if=/dev/zero bs=1M count=0 seek=64 of=$(IMAGE_NAME).hdd
	parted -s $(IMAGE_NAME).hdd mklabel gpt
	parted -s $(IMAGE_NAME).hdd mkpart ESP fat32 2048s 100%
	parted -s $(IMAGE_NAME).hdd set 1 esp on
	./limine/limine bios-install $(IMAGE_NAME).hdd
	sudo losetup -Pf --show $(IMAGE_NAME).hdd >loopback_dev
	sudo mkfs.fat -F 32 `cat loopback_dev`p1
	mkdir -p img_mount
	sudo mount `cat loopback_dev`p1 img_mount
	sudo cp -vr sysroot/* img_mount/
	sync
	sudo umount img_mount
	sudo losetup -d `cat loopback_dev`
	rm -rf loopback_dev img_mount

.PHONY: kernel-docs
kernel-docs:
	cd include; doxygen

.SILENT: xconfig menuconfig

.PHONY: menuconfig
menuconfig:
	kconfig-mconf Kconfig
	echo "Generating config.h..."
	python3 scripts/genconfig.py
	echo "done"

.PHONY: xconfig
xconfig:
	kconfig-qconf Kconfig
	echo "Generating config.h..."
	python3 scripts/genconfig.py
	echo "done"

.PHONY: clean
clean:
	rm -rf iso_root $(IMAGE_NAME).iso $(IMAGE_NAME).hdd docs/html
	$(MAKE) -C kernel clean
	$(MAKE) -C modules clean
	$(MAKE) -C programs clean

.PHONY: distclean
distclean: clean
	rm -rf limine ovmf sysroot toolchain mlibc hosttools .config .config.old
	$(MAKE) -C kernel distclean
