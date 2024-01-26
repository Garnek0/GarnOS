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
	qemu-system-x86_64 -m 2G -cdrom $(IMAGE_NAME).iso -boot d
	
.PHONY: run-uefi
run-uefi: ovmf $(IMAGE_NAME).iso
	qemu-system-x86_64 -M q35 -m 2G -bios ovmf/OVMF.fd -cdrom $(IMAGE_NAME).iso -boot d

.PHONY: run-hdd
run-hdd: $(IMAGE_NAME).hdd
	qemu-system-x86_64 -m 2G -hda $(IMAGE_NAME).hdd -d int -M q35 -M smm=off -no-reboot

.PHONY: run-hdd-uefi
run-hdd-uefi: ovmf $(IMAGE_NAME).hdd
	qemu-system-x86_64 -M q35 -m 2G -bios ovmf/OVMF.fd -hda $(IMAGE_NAME).hdd

ovmf:
	mkdir -p ovmf
	cd ovmf && curl -Lo OVMF.fd https://retrage.github.io/edk2-nightly/bin/RELEASEX64_OVMF.fd

limine:
	git clone https://github.com/limine-bootloader/limine.git --branch=v6.x-branch-binary --depth=1
	$(MAKE) -C limine CC="$(HOST_CC)"
	cp -v limine.cfg limine/limine-bios.sys limine/limine-bios-cd.bin limine/limine-uefi-cd.bin sysroot
	cp -v limine/BOOTX64.EFI sysroot/efi/boot/BOOTX64.EFI
	
.PHONY: kernel
kernel:
	$(MAKE) -C kernel

.PHONY: modules
modules:
	$(MAKE) -C modules

.PHONY: programs
programs:
	$(MAKE) -C programs

sysroot:
	rm -rf sysroot

	mkdir sysroot
	mkdir sysroot/bin
	# mkdir sysroot/lib
	# mkdir sysroot/include
	mkdir sysroot/efi
	mkdir sysroot/efi/boot

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

.PHONY: clean
clean:
	rm -rf iso_root $(IMAGE_NAME).iso $(IMAGE_NAME).hdd
	$(MAKE) -C kernel clean
	$(MAKE) -C modules clean
	$(MAKE) -C programs clean

.PHONY: distclean
distclean: clean
	rm -rf limine ovmf sysroot
	$(MAKE) -C kernel distclean