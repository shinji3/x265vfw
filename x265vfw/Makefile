##############################################################################
#
# Makefile for x265 VFW driver
#
# Author: XviD project:
#            - Milan Cutka <cutka at szm.sk>,
#            - Edouard Gomez <ed.gomez at free.fr>
#            - Peter Ross <pross@xvid.org>
# Ported to x264 by Laurent Aimar <fenrir@via.ecp.fr>
# Ported to x265 by Attila Padar <mpxplay@freemail.hu>
#
##############################################################################

-include x265vfw_config.mak

# Current dir
DIR_CUR = $(shell pwd)

# Path to include files (library and src)
DIR_SRC = $(DIR_CUR)

ifeq ($(FFMPEG_DIR)x,x)
ifeq ($(FFMPEG_NAME)x,x)
ifneq ($(wildcard $(DIR_CUR)/../ffmpeg/*)x,x)
FFMPEG_DIR = $(DIR_CUR)/../ffmpeg
FFMPEG_NAME = ffmpeg
else
ifneq ($(wildcard $(DIR_CUR)/../libav/*)x,x)
FFMPEG_DIR = $(DIR_CUR)/../libav
FFMPEG_NAME = libav
endif
endif
else
ifneq ($(wildcard $(DIR_CUR)/../$(FFMPEG_NAME)/*)x,x)
FFMPEG_DIR = $(DIR_CUR)/../$(FFMPEG_NAME)
endif
endif
endif

ifeq ($(FFMPEG_NAME)x,x)
FFMPEG_NAME = ffmpeg
endif

-include config.mak

ifeq ($(SYS_ARCH),X86_64)
# Dll to build
DLL = x265vfw64.dll
else
# Dll to build
DLL = x265vfw.dll
endif

##############################################################################
# CFLAGS
##############################################################################

# Constants which should not be modified
CFLAGS += -D_WIN32_IE=0x0501

##############################################################################
# Compiler flags for linking stage
##############################################################################

VFW_LDFLAGS = -static

##############################################################################
# Conditional options
##############################################################################

CFLAGS += "-I$(FFMPEG_DIR)"
VFW_LDFLAGS += "-L$(FFMPEG_DIR)/libavcodec" -lavcodec
VFW_LDFLAGS += "-L$(FFMPEG_DIR)/libavutil" -lavutil
VFW_LDFLAGS += "-L$(FFMPEG_DIR)/libswscale" -lswscale
VFW_LDFLAGS += $(EXTRALIBS)

# Sources
SRC_C = codec.c driverproc.c

# Muxers
CONFIG =
ifneq ($(wildcard config.h)x,x)
CONFIG := $(shell cat config.h)
endif

##############################################################################
# Rules
##############################################################################

OBJECTS  = $(SRC_C:.c=.obj)

.SUFFIXES:
.SUFFIXES: .obj .rc .c

DIR_BUILD = $(DIR_CUR)/bin
VPATH = $(DIR_SRC):$(DIR_BUILD)

.PHONY: all clean distclean

all: $(DLL)

ifneq ($(wildcard .depend)x,x)
include .depend
endif

%.obj: %.rc
	@echo " W: $(@D)/$(<F)"
	@mkdir -p "$(DIR_BUILD)/$(@D)"
	@$(RC) \
	--input-format=rc \
	--output-format=coff \
	-o "$(DIR_BUILD)/$@" $<

%.obj: %.c
	@echo " C: $(@D)/$(<F)"
	@mkdir -p "$(DIR_BUILD)/$(@D)"
	@$(CC) $(CFLAGS) -c -o "$(DIR_BUILD)/$@" $<

.depend: config.mak
	@rm -f .depend
	@$(foreach SRC, $(SRC_C), $(CC) $(CFLAGS) $(SRC) -MT $(SRC:%.c=%.obj) -MM -g0 1>> .depend;)

$(DLL): .depend config.mak config.h $(OBJECTS)
	@echo " L: $(@F)"
	@mkdir -p "$(DIR_BUILD)"
	@cp -f "$(DIR_SRC)/x265vfw.bat" "$(DIR_BUILD)/x265vfw.bat"
	@cp -f "$(DIR_SRC)/x265vfw.inf" "$(DIR_BUILD)/x265vfw.inf"
	@cp -f "$(DIR_SRC)/driverproc.def" "$(DIR_BUILD)/driverproc.def"
	@cd "$(DIR_BUILD)" && \
	$(CC) \
	-shared -Wl,-dll,--out-implib,$@.a,--enable-stdcall-fixup \
	-o $@ \
	$(OBJECTS) driverproc.def \
	$(VFW_LDFLAGS) $(LDFLAGS) -lgdi32 -lwinmm -lcomdlg32 -lcomctl32

clean:
	@echo " Cl: Object files and target lib"
	@rm -rf "$(DIR_BUILD)"
	@echo " Cl: .depend"
	@rm -f .depend

distclean: clean
	@echo " Cl: config.mak"
	@rm -f "$(DIR_CUR)/config.mak"
	@echo " Cl: config.h"
	@rm -f "$(DIR_CUR)/config.h"
