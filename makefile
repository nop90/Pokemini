# PokeMini Makefile for 3DS

#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

include $(DEVKITARM)/3ds_rules

POKEROOT = ../../

#---------------------------------------------------------------------------------
# TARGET is the name of the output
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# INCLUDES is a list of directories containing extra header files
# MAXMOD_SOUNDBANK contains a directory of music and sound effect files
#---------------------------------------------------------------------------------
TARGET		:=	PokeMini
BUILD		:=	build
SOURCES		:=	$(POKEROOT)source $(POKEROOT)resource $(POKEROOT)freebios $(POKEROOT)dependencies/minizip $(POKEROOT)dependencies/zlib $(POKEROOT)sourcex $(POKEROOT)platform/3ds/source
DATA		:=	data
INCLUDES	:=	$(POKEROOT)source $(POKEROOT)resource $(POKEROOT)freebios $(POKEROOT)sourcex $(POKEROOT)dependencies/zlib $(POKEROOT)dependencies/minizip $(POKEROOT)dependencies/zlib $(POKEROOT)sourcex $(POKEROOT)platform/3ds

APP_TITLE		:= PokeMini 3DS
APP_DESCRIPTION	:= PokeMini emulator for 3DS
APP_AUTHOR		:= JustBurn (Port to 3DS by nop90)

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------
ARCH	:=	-march=armv6k -mtune=mpcore -mfloat-abi=hard

CFLAGS	:=	-g -Wall -O2 -mword-relocations \
			-fomit-frame-pointer -ffast-math \
			$(ARCH) 

CFLAGS	+=	$(INCLUDE) -DARM11 -D_3DS

CXXFLAGS	:= $(CFLAGS) -fno-rtti -fno-exceptions -std=gnu++11

ASFLAGS	:=	-g $(ARCH)
LDFLAGS	=	-specs=3dsx.specs -g $(ARCH) -Wl,-Map,$(notdir $*.map)

#---------------------------------------------------------------------------------
# any extra libraries we wish to link with the project (order is important)
#---------------------------------------------------------------------------------
LIBS	:= -lsf2d -lctru -lm

#---------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level containing
# include and lib
#---------------------------------------------------------------------------------
LIBDIRS	:= $(CTRULIB) 

#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------

export OUTPUT	:=	$(CURDIR)/$(TARGET)

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
					$(foreach dir,$(DATA),$(CURDIR)/$(dir))

export DEPSDIR	:=	$(CURDIR)/$(BUILD)

# platform/3DS/PokeMini_3DS.c	\


# Only include what's really needed
CFILES		:= \
 platform/3ds/source/PokeMini_3DS.c	\
 platform/3ds/source/PokeMini_3DS_sound.c \
 platform/3ds/source/Video.c	\
 platform/3ds/source/Video_x1.c	\
 platform/3ds/source/Video_x2.c	\
 platform/3ds/source/UI.c	\
 platform/3ds/source/MinxAudio.c	\
 freebios/freebios.c	\
 source/PMCommon.c	\
 source/PokeMini.c	\
 source/Multicart.c	\
 source/Hardware.c	\
 source/CommandLine.c	\
 source/MinxCPU.c	\
 source/MinxCPU_XX.c	\
 source/MinxCPU_CE.c	\
 source/MinxCPU_CF.c	\
 source/MinxCPU_SP.c \
 source/MinxTimers.c	\
 source/MinxIO.c	\
 source/MinxIRQ.c	\
 source/MinxPRC.c	\
 source/MinxColorPRC.c	\
 source/MinxLCD.c	\
 source/Joystick.c	\
 dependencies/minizip/unzip.c	\
 dependencies/minizip/ioapi.c	\
 dependencies/zlib/adler32.c	\
 dependencies/zlib/crc32.c	\
 dependencies/zlib/inffast.c	\
 dependencies/zlib/inflate.c	\
 dependencies/zlib/inftrees.c	\
 dependencies/zlib/zutil.c	\
 sourcex/Zip64comp.c	\
 resource/PokeMini_ColorPal.c	\
 resource/PokeMini_Font12.c	\
 resource/PokeMini_Icons12.c	\
 resource/PokeMini_BG3.c

CFILES		:=	$(notdir $(CFILES))
# CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
BINFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))
PNGFILES	:=	$(foreach dir,$(GRAPHICS),$(notdir $(wildcard $(dir)/*.png)))

#---------------------------------------------------------------------------------
# use CXX for linking C++ projects, CC for standard C
#---------------------------------------------------------------------------------
ifeq ($(strip $(CPPFILES)),)
#---------------------------------------------------------------------------------
	export LD	:=	$(CC)
#---------------------------------------------------------------------------------
else
#---------------------------------------------------------------------------------
	export LD	:=	$(CXX)
#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------

export OFILES	:=	$(addsuffix .o,$(BINFILES)) \
			$(PNGFILES:.png=.o) \
			$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)

export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
			$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
			-I$(CURDIR)/$(BUILD)

export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib)

ifeq ($(strip $(ICON)),)
	icons := $(wildcard *.png)
	ifneq (,$(findstring $(TARGET).png,$(icons)))
		export APP_ICON := $(CURDIR)/$(TARGET).png
	else
		ifneq (,$(findstring icon.png,$(icons)))
			export APP_ICON := $(CURDIR)/icon.png
		endif
	endif
else
	export APP_ICON := $(TOPDIR)/$(ICON)
endif

ifeq ($(strip $(NO_SMDH)),)
	export _3DSXFLAGS += --smdh=$(CURDIR)/$(TARGET).smdh
endif

.PHONY: $(BUILD) clean all

#---------------------------------------------------------------------------------
all: $(BUILD)

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

#---------------------------------------------------------------------------------
clean:
	@echo clean ...
	@rm -fr $(BUILD) $(TARGET).3dsx $(OUTPUT).smdh $(TARGET).elf $(TARGET)-strip.elf $(TARGET).cia $(TARGET).3ds
#---------------------------------------------------------------------------------
$(TARGET)-strip.elf: $(BUILD)
	@$(STRIP) $(TARGET).elf -o $(TARGET)-strip.elf
#---------------------------------------------------------------------------------
cci: $(TARGET)-strip.elf
	@makerom -f cci -rsf resources/$(TARGET).rsf -target d -exefslogo -elf $(TARGET)-strip.elf -o $(TARGET).3ds
	@echo "built ... 3ds"
#---------------------------------------------------------------------------------
cia: $(TARGET)-strip.elf
	@makerom -f cia -o $(TARGET).cia -elf $(TARGET)-strip.elf -rsf resources/$(TARGET).rsf -icon resources/icon.bin -banner resources/banner.bin -exefslogo -target t
	@echo "built ... cia"
#---------------------------------------------------------------------------------
else

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
ifeq ($(strip $(NO_SMDH)),)
$(OUTPUT).3dsx	:	$(OUTPUT).elf $(OUTPUT).smdh
else
$(OUTPUT).3dsx	:	$(OUTPUT).elf
endif

$(OUTPUT).elf	:	$(OFILES)

#---------------------------------------------------------------------------------
%.bin.o	:	%.bin
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	$(bin2o)

#---------------------------------------------------------------------------------
%.s %.h	: %.png %.grit
#---------------------------------------------------------------------------------
	grit $< -fts -o$*

-include $(DEPSDIR)/*.d

#---------------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------------
