# GarnOS

![C](https://img.shields.io/badge/c-%2300599C.svg?style=for-the-badge&logo=c&logoColor=white&color=orange)
![](https://tokei.rs/b1/github/garnek0/garnos?type=c,c++,cheader,assembly,assemblyGAS,Rust&style=for-the-badge&color=orange)

<img width="256" height="256" src="GarnOS.svg">

GarnOS is a simple, open-source, 64-bit hobby operating system.

The main goals of GarnOS are compatibility and modularity.

## Notable Features

- Terminal Emulator
- Module Loader
- Userspace
- Device Manager

# Building & Running

## Dependencies
- GCC and Binutils
- At least a minimum toolset to build a cross-toolchain
- QEMU
- Xorriso (for ISO images)
- GNU Parted
- Git
- make/gmake and cmake (cmake is required by mlibc)

## Building

First of all, you need to build the toolchain:
```
make all-toolchain
``` 

Then, in the root directory, run:
```
make all # ISO Image
make all-hdd # HDD Image
```
NOTE: The ISO image will probably not work. Use the HDD image instead!

You may also run the OS directly, as described below, since the `run-*` targets also build the OS in case it hasn't been built already.

## Running
In the root directory, run:
```
make run # Run using ISO Image
make run-hdd # Run using HDD Image
```

You can also run GarnOS in an EFI environment by appending `-uefi` to the commands shown above

For debugging, it is recommended to append `DEBUG=y"` to whatever make command you're using.

## License
See LICENSE for details.

## Known Bugs

- Disk access doesnt work properly on "large" drives (> ~512 MiB).
- Disk access is EXTREMELY slow. It's so slow it takes 2-4 seconds to load a simple program such as `ls`.
- AHCI driver doesn't work on real hardware.
- mlibc sometimes panics when running user programs.

## Support

**General**: PCI \
**Input**: PS/2 Keyboards \
**Storage**: IDE, AHCI \
**File Systems**: FAT32 \
**Graphics**: Standard Graphics

<img src="shell.png">

## Acknowledgements

- Limine Bootloader (mintsuki et al.)
- uACPI (d-tatianin and DataBeaver)
