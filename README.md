# GarnOS

![](https://tokei.rs/b1/github/Garnek0/GarnOS)

GarnOS is A Simple, Open-Source, Hobby Operating System.

IMPORTANT: I have been very busy lately due to school and things are probably not going to get easier any time soon. I have been trying to allocate my very limited time in such a way that I would be able to release at least one version per week, but now even that isn't going to work anymore. I'll do my best to keep this project alive.

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
See LICENSE for details.

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

## Hardware Support

**General**: PCI \
**Input**: PS/2 Keyboards \
**Storage**: IDE \
**Graphics**: Standard Graphics