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

ifeq ($(X265_DIR)x,x)
X265_DIR = $(DIR_CUR)/../x265-master
X265_BIN_DIR = $(X265_DIR)/build/msys
endif

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

ifeq ($(HAVE_FFMPEG)x,x)
ifeq ($(FFMPEG_DIR)x,x)
HAVE_FFMPEG = no
else
HAVE_FFMPEG = yes
endif
endif

-include config.mak

ifeq ($(SYS_ARCH),X86_64)
# Dll to build
DLL = x265vfw64.dll

# Installer script
INST_NSI = x265vfw64.nsi

# Installer executable
INST_EXE = x265vfw64.exe
else
# Dll to build
DLL = x265vfw.dll

# Installer script
INST_NSI = x265vfw.nsi

# Installer executable
INST_EXE = x265vfw.exe
endif

# cross-prefix
ifeq ($(CROSS_PREFIX)x,x)
CROSS_PREFIX = $(shell echo "$(RANLIB)" | grep 'ranlib$$' | sed -e 's/ranlib$$//')
endif

# Alias
ifeq ($(WINDRES)x,x)
WINDRES = $(CROSS_PREFIX)windres
endif

##############################################################################
# CFLAGS
##############################################################################

# Constants which should not be modified
CFLAGS += -D_WIN32_IE=0x0501
CFLAGS += "-I$(X265_DIR)/source" "-I$(X265_BIN_DIR)"

##############################################################################
# Compiler flags for linking stage
##############################################################################

VFW_LDFLAGS = -static "-L$(X265_BIN_DIR)" -lx265.dll

##############################################################################
# Conditional options
##############################################################################

RESFLAGS =
ifeq ($(HAVE_FFMPEG),yes)
RESFLAGS += -DHAVE_FFMPEG
RESFLAGS += "-DFFMPEG_LOGO=$(FFMPEG_NAME).bmp"
CFLAGS += -DHAVE_FFMPEG
CFLAGS += "-I$(FFMPEG_DIR)"
#VFW_LDFLAGS += "-L$(FFMPEG_DIR)/libavformat" -lavformat
VFW_LDFLAGS += "-L$(FFMPEG_DIR)/libavcodec" -lavcodec
VFW_LDFLAGS += "-L$(FFMPEG_DIR)/libavutil" -lavutil
VFW_LDFLAGS += "-L$(FFMPEG_DIR)/libswscale" -lswscale
VFW_LDFLAGS += $(EXTRALIBS)
endif

ifneq ($(wildcard version.h)x,x)
RESFLAGS += -DHAVE_VERSION
RESFLAGS += "-DDLL_NAME=$(DLL)"
endif

# Sources
SRC_C = codec.c config.c csp.c driverproc.c
SRC_RES = resource.rc

# Muxers
CONFIG =
ifneq ($(wildcard config.h)x,x)
CONFIG := $(shell cat config.h)
endif

SRCS_LSMASH = common/alloc.c common/bits.c common/bytes.c common/list.c \
              common/multibuf.c common/osdep.c common/utils.c \
              codecs/a52.c codecs/alac.c codecs/description.c codecs/dts.c \
              codecs/h264.c codecs/hevc.c codecs/id.c codecs/mp4sys.c \
              codecs/mp4a.c codecs/vc1.c codecs/wma.c \
              importer/a52_imp.c importer/adts_imp.c importer/als_imp.c \
              importer/amr_imp.c importer/dts_imp.c importer/importer.c \
              importer/isobm_imp.c importer/mp3_imp.c importer/nalu_imp.c \
              importer/vc1_imp.c importer/wave_imp.c \
              core/box.c core/chapter.c core/file.c core/fragment.c \
              core/isom.c core/meta.c core/print.c core/read.c \
              core/summary.c core/timeline.c core/write.c

CFLAGS += -Ioutput/L-SMASH

#TODO: fix file output writers
#SRC_C += output/raw.c
#SRC_C += output/matroska.c output/matroska_ebml.c
#SRC_C += output/flv.c output/flv_bytestream.c
#SRC_C += output/mp4_lsmash.c
#SRC_C += $(addprefix output/L-SMASH/, $(SRCS_LSMASH))

ifeq ($(HAVE_FFMPEG),yes)
#SRC_C += output/avi.c
endif

# MinGW64 bugfix
ifeq ($(SYS_ARCH),X86_64)
VFW_LDFLAGS += -liconv
endif

##############################################################################
# Rules
##############################################################################

OBJECTS  = $(SRC_C:.c=.obj)
OBJECTS += $(SRC_RES:.rc=.obj)

.SUFFIXES:
.SUFFIXES: .obj .rc .c

DIR_BUILD = $(DIR_CUR)/bin
VPATH = $(DIR_SRC):$(DIR_BUILD)

.PHONY: all clean distclean build-installer

# all: $(DLL)
all: build-installer

ifneq ($(wildcard .depend)x,x)
include .depend
endif

# Resource dependence manually
resource.obj: resource.rc resource.h x265vfw_config.h
ifneq ($(wildcard version.h)x,x)
resource.obj: version.h
endif

%.obj: %.rc
	@echo " W: $(@D)/$(<F)"
	@mkdir -p "$(DIR_BUILD)/$(@D)"
	@$(WINDRES) $(RESFLAGS) \
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

##############################################################################
# Builds the NSIS installer script for Windows.
# NSIS 2.x is required and makensis.exe should be in the path
##############################################################################

$(INST_EXE): $(DLL)
	@cp -f "$(DIR_SRC)/installer/$(INST_NSI)" "$(DIR_BUILD)/$(INST_NSI)"
	@cp -f "$(DIR_SRC)/installer/x265vfw.ico" "$(DIR_BUILD)/x265vfw.ico"
	@cp -f "$(X265_BIN_DIR)/libx265.dll" "$(DIR_BUILD)/libx265.dll"
	@makensis "$(DIR_BUILD)/$(INST_NSI)"
	@rm -f "$(DIR_BUILD)/$(INST_NSI)"
	@rm -f "$(DIR_BUILD)/x265vfw.ico"
	@rm -f "$(DIR_BUILD)/libx265.dll"

build-installer: $(INST_EXE)