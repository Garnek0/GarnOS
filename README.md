# GarnOS

![](https://tokei.rs/b1/github/Garnek0/GarnOS)

GarnOS is A Simple, Open-Source, Hobby Operating System.

## Notable Features

- Terminal Emulator
- Module Loader
- Userspace

# Building & Running

## Building

In the root directory, run:
```
make all
```
to build the iso, or:
```
make all-hdd
```
to get a hdd image.

NOTE: I dont guarantee the iso image will work, it is recommended to use the hdd image instead!

## Running
In the root directory, run:
```
make run
```
to run the iso version in QEMU, or
```
make run-hdd
```
to run the hdd image.

You can also run GarnOS in an EFI environment by appending `-uefi` to the commands showed above

NOTE: At the time of writing this, the OVMF Nightly precompiled binary is broken and will not work. If that is the case, you will have to run GarnOS normally. (in BIOS mode)

# License
See LICENSE.md for details.

# Milestones

- [x] RTC
- [x] SMP
- [x] APIC, I/O APIC
- [x] Timer
- [x] PCI Driver
- [ ] AHCI driver
- [x] IDE driver
- [x] FAT Driver
- [ ] Ext2 Driver
- [x] Userland
- [x] Scheduler
- [x] Processes and Threads
- [x] Syscalls
- [ ] Driver interface (Custom or UDI)
- [ ] GUI
- [ ] USB Driver
- [ ] Sound
- [ ] Networking

## Known Bugs

- APIC implementation doesn't work properly on some systems.
- Painfully slow file access due to lack of proper file caching. (File data is read directly from the disk each time a file operation (read, write) is made)
- Weird behaviour with kheap allocation blocks.

## Hardware Support

**General**: PCI \
**Input**: PS/2 Keyboards \
**Storage**: IDE \
**Graphics**: none

# Acknowledgments

- toaruos (module loader)