#
# Makefile for Virtual Jaguar core library
#
# by James L. Hammons
#
# This software is licensed under the GPL v3 or any later version. See the
# file GPLv3 for details. ;-)
#

# Figure out which system we're compiling for, and set the appropriate variables

OSTYPE := $(shell uname -a)

ifeq "$(findstring Msys,$(OSTYPE))" "Msys"			# Win32

SYSTYPE    := __GCCWIN32__
SDLLIBTYPE := --libs

else ifeq "$(findstring Darwin,$(OSTYPE))" "Darwin"	# Should catch both 'darwin' and 'darwin7.0'

SYSTYPE    := __GCCUNIX__ -D__THINK_STUPID__
SDLLIBTYPE := --static-libs

else ifeq "$(findstring Linux,$(OSTYPE))" "Linux"		# Linux

SYSTYPE    := __GCCUNIX__
SDLLIBTYPE := --libs

else											# ???

$(error OS TYPE UNDETECTED)

endif

# Set vars for libcdio
ifneq "$(shell pkg-config --silence-errors --libs libcdio)" ""
HAVECDIO := -DHAVE_LIB_CDIO
CDIOLIB  := -lcdio
else
HAVECDIO :=
CDIOLIB  :=
endif

CC      := gcc
LD      := gcc
AR      := ar
ARFLAGS := -rs

# Note that we use optimization level 2 instead of 3--3 doesn't seem to gain much over 2
CFLAGS  := -MMD -O2 -ffast-math -fomit-frame-pointer `sdl-config --cflags` -D$(SYSTYPE)
CXXFLAGS  := -MMD -O2 -ffast-math -fomit-frame-pointer `sdl-config --cflags` -D$(SYSTYPE)

INCS := -I./src

OBJS := \
	obj/blitter.o      \
	obj/cdintf.o       \
	obj/cdrom.o        \
	obj/crc32.o        \
	obj/dac.o          \
	obj/dsp.o          \
	obj/eeprom.o       \
	obj/event.o        \
	obj/file.o         \
	obj/filedb.o       \
	obj/gpu.o          \
	obj/jagbios.o      \
	obj/jagcdbios.o    \
	obj/jagdevcdbios.o \
	obj/jagstub1bios.o \
	obj/jagstub2bios.o \
	obj/jagdasm.o      \
	obj/jaguar.o       \
	obj/jerry.o        \
	obj/joystick.o     \
	obj/log.o          \
	obj/memory.o       \
	obj/mmu.o          \
	obj/op.o           \
	obj/settings.o     \
	obj/state.o        \
	obj/tom.o          \
	obj/universalhdr.o \
	obj/unzip.o        \
	obj/wavetable.o

# Targets for convenience sake, not "real" targets
.PHONY: clean

all: obj obj/libjaguarcore.a
	@echo "Done!"

obj:
	@mkdir obj

# Library rules (might not be cross-platform compatible)
obj/libjaguarcore.a: $(OBJS) 
	@$(AR) $(ARFLAGS) obj/libjaguarcore.a $(OBJS)

# Main source compilation (implicit rules)...

obj/%.o: src/%.c
	@echo -e "\033[01;33m***\033[00;32m Compiling $<...\033[00m"
	@$(CC) $(CFLAGS) $(INCS) -c $< -o $@

obj/%.o: src/%.cpp
	@echo -e "\033[01;33m***\033[00;32m Compiling $<...\033[00m"
	@$(CC) $(CXXFLAGS) $(INCS) -c $< -o $@

-include obj/*.d
