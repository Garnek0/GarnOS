MODULE_NAME := rtc

# CFLAGS, CPPFLAGS, LD, CC are already defined

CFILES := $(shell find -L . -type f -name '*.c')
OBJ := $(CFILES:.c=.c.o)

.PHONY: all
all:
	$(CC) $(CFLAGS) $(CPPFLAGS) $(CFILES) -o $(MODULE_NAME).sys -r
	cp $(MODULE_NAME).sys ..
	rm $(MODULE_NAME).sys
