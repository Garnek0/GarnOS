# Nuke built-in rules and variables.
override MAKEFLAGS += -rR

# This is the name that our final kernel executable will have.
# Change as needed.
override KERNEL := kernel.elf

# Convenience macro to reliably declare user overridable variables.
override DEFAULT_VAR = $(if $(filter $(origin $(1)),default undefined),$(eval override $(1) := $(2)))

# Target Architecture. Default is x86_64
$(call DEFAULT_VAR,ARCH,x86_64)


# It is highly recommended to use a custom built cross toolchain to build a kernel.
# We are only using "cc" as a placeholder here. It may work by using
# the host system's toolchain, but this is not guaranteed.

include ../.config

override DEFAULT_CC := ../hosttools/bin/x86_64-pc-garn-mlibc-gcc
$(eval $(call DEFAULT_VAR,CC,$(DEFAULT_CC)))

# Same thing for "ld" (the linker).
override DEFAULT_LD := ../hosttools/bin/x86_64-pc-garn-mlibc-ld
$(eval $(call DEFAULT_VAR,LD,$(DEFAULT_LD)))

# User controllable C flags.
ifdef DEBUG
override DEFAULT_CFLAGS := -fsanitize=undefined -g -O2 -pipe
$(eval $(call DEFAULT_VAR,CFLAGS,$(DEFAULT_CFLAGS)))
else
override DEFAULT_CFLAGS := -O2 -pipe
$(eval $(call DEFAULT_VAR,CFLAGS,$(DEFAULT_CFLAGS)))
endif

# User controllable C preprocessor flags. We set none by default.
override DEFAULT_CPPFLAGS :=
$(eval $(call DEFAULT_VAR,CPPFLAGS,$(DEFAULT_CPPFLAGS)))

# User controllable nasm flags.
override DEFAULT_NASMFLAGS := -F dwarf -g
$(eval $(call DEFAULT_VAR,NASMFLAGS,$(DEFAULT_NASMFLAGS)))

# User controllable linker flags. We set none by default.
override DEFAULT_LDFLAGS :=
$(eval $(call DEFAULT_VAR,LDFLAGS,$(DEFAULT_LDFLAGS)))

# Internal C flags that should not be changed by the user.
override CFLAGS += \
	-Wall \
	-Wextra \
	-Wno-unused-parameter \
	-std=gnu11 \
	-ffreestanding \
	-fno-stack-protector \
	-fno-stack-check \
	-fno-lto \
	-fPIE \
	-m64 \
	-march=x86-64 \
	-mabi=sysv \
	-mno-80387 \
	-mno-mmx \
	-mno-sse \
	-mno-sse2 \
	-mno-red-zone

# Internal C preprocessor flags that should not be changed by the user.
override CPPFLAGS := \
	-I. \
	-I../ \
	-I../include \
	-Iklibc/include \
	$(CPPFLAGS) \
	-MMD \
	-MP

# Internal linker flags that should not be changed by the user.
override LDFLAGS += \
	-m elf_x86_64 \
	-nostdlib \
	-static \
	-pie \
	--no-dynamic-linker \
	-z text \
	-z max-page-size=0x1000 \
	-T linker.ld

# Internal nasm flags that should not be changed by the user.
override NASMFLAGS += \
	-Wall \
	-f elf64

# Use "find" to glob all *.c, *.S, and *.asm files in the tree and obtain the
# object and header dependency file names.
override CFILES := $(shell find -L . -type f -name '*.c' -not -path "./arch/*")
override ASFILES := $(shell find -L . -type f -name '*.S' -not -path "./arch/*")
override NASMFILES := $(shell find -L . -type f -name '*.asm' -not -path "./arch/*")

override CFILES += $(shell find -L ./arch/$(ARCH) -type f -name '*.c')
override ASFILES += $(shell find -L ./arch/$(ARCH) -type f -name '*.S')
override NASMFILES += $(shell find -L ./arch/$(ARCH) -type f -name '*.asm')

override OBJ := $(CFILES:.c=.c.o) $(ASFILES:.S=.S.o) $(NASMFILES:.asm=.asm.o)
override HEADER_DEPS := $(CFILES:.c=.c.d) $(ASFILES:.S=.S.d)

# Default target.
.PHONY: all
all: $(KERNEL)

limine.h:
	curl -Lo $@ https://github.com/limine-bootloader/limine/raw/trunk/limine.h

# Link rules for the final kernel executable.
$(KERNEL): $(OBJ)
	$(LD) $(OBJ) $(LDFLAGS) -o $@
	cp $@ ../sysroot/$@

# Include header dependencies.
-include $(HEADER_DEPS)

# Compilation rules for *.c files.
%.c.o: %.c limine.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

# Compilation rules for *.S files.
%.S.o: %.S limine.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

# Compilation rules for *.asm (nasm) files.
%.asm.o: %.asm
	nasm $(NASMFLAGS) $< -o $@

# Remove object files and the final executable.
.PHONY: clean
clean:
	rm -rf $(KERNEL) $(OBJ) $(HEADER_DEPS)

.PHONY: distclean
distclean: clean
	rm -f limine.h
