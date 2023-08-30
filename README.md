# GarnOS

GarnOS is A Simple, Open-Source, Hobby Operating System.

## Notable Features

- Terminal Emulator
- Module Loader
- Limine™

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

- [ ] RTC
- [ ] SMP
- [ ] APIC, I/O APIC and MSIs
- [ ] Timer (PIT or APIC Timer)
- [ ] Syscalls, userland and ELF executables
- [ ] Driver interface (Custom or UDI)
- [ ] PCI Enumeration
- [ ] AHCI driver
- [ ] IDE driver
- [ ] FAT32 Driver
- [ ] GUI
- [ ] Scheduler, multiprocessing
- [ ] USB Driver
- [ ] SB16 Driver
- [ ] Networking

# Acknowledgments

- toaruos