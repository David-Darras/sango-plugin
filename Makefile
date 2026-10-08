#---------------------------------------------------------------------------------
# Sango Plugin - build file
#
# Usage (from the devkitPro shell):
#   make                  builds all the products of the selected game
#   make overlay          builds one product (overlay, kaizo or undertow)
#   make GAME=XY          builds for Pokemon X instead of Alpha Sapphire
#   make run-overlay      builds one product, copies it to the SD card of the
#                         emulator and starts the game (needs config.mk)
#   make clean            removes all the build output
#
# Put your local settings (emulator, game dump, SD card) in config.mk.
# Copy config.mk.example to config.mk to start. Git ignores config.mk.
#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

export TOPDIR ?= $(CURDIR)
include $(DEVKITARM)/3ds_rules

# Local settings. The file is optional: only `make run-<product>` needs it.
-include $(TOPDIR)/config.mk

PLUGIN_VERSION  := 6.0.0
PLUGIN_CREATOR  := ZettaD
PLUGIN_NAME     := Sango

#---------------------------------------------------------------------------------
# The target game.
#   ORAS: Pokemon Alpha Sapphire v1.4 (title id 000400000011C500), the main game.
#   XY:   Pokemon X v1.5 (title id 0004000000055D00), experimental, overlay only.
#---------------------------------------------------------------------------------
GAME      ?= ORAS
ifeq ($(GAME),XY)
GAME_DEFINE   := -DGAME_XY
GAME_PRODUCTS := overlay
TITLE_ID      := 0004000000055D00
GAME_PATH     ?= $(XY_GAME_PATH)
else
GAME_DEFINE   := -DGAME_ORAS
GAME_PRODUCTS := overlay kaizo undertow
TITLE_ID      := 000400000011C500
GAME_PATH     ?= $(ORAS_GAME_PATH)
endif

# The plugin folder of the game on the SD card.
ifneq ($(strip $(SDMC)),)
DEST      ?= $(SDMC)/luma/plugins/$(TITLE_ID)
endif

CTRPFLIB	?=	$(DEVKITPRO)/libctrpf

#---------------------------------------------------------------------------------
# One library, several plugins built on it. `make` builds them all,
# `make overlay` / `make kaizo` just one; each gets its own build directory.
#
# To add your own ROM hack, copy one product block below, change the name
# and add the name to GAME_PRODUCTS above. See docs/tutorials/01-create-your-rom-hack.md.
#---------------------------------------------------------------------------------
PRODUCTS	:=	$(GAME_PRODUCTS)

LIB_SOURCES	:=	lib/src \
				lib/src/core \
				lib/src/net \
				lib/src/battle \
				lib/src/overworld \
				lib/src/pokemon \
				lib/src/savedata \
				lib/src/renderer \
				lib/src/script \
				lib/src/ui \
				lib/src/ui/widget \
				lib/src/ui/page
LIB_INCLUDES	:=	lib/include \
				lib/src

# sango_plugin.3gx: every page of the library under one menu
overlay_TARGET	:=	sango_plugin
overlay_SOURCES	:=	$(LIB_SOURCES) overlay/src
overlay_INCLUDES	:=	$(LIB_INCLUDES) overlay/include
overlay_PSF		:=	overlay/sango_plugin.plgInfo

# sango_kaizo.3gx: the ROM hack
kaizo_TARGET	:=	sango_kaizo
kaizo_SOURCES	:=	$(LIB_SOURCES) kaizo/src
kaizo_INCLUDES	:=	$(LIB_INCLUDES) kaizo/include
kaizo_PSF		:=	kaizo/sango_kaizo.plgInfo

# sango_undertow.3gx: the ROM hack where you play a Team Aqua grunt
undertow_TARGET	:=	sango_undertow
undertow_SOURCES	:=	$(LIB_SOURCES) undertow/src
undertow_INCLUDES	:=	$(LIB_INCLUDES) undertow/include
undertow_PSF		:=	undertow/sango_undertow.plgInfo

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------
ARCH	:=	-march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft

CFLAGS	:=	-mword-relocations \
 			-ffunction-sections -fdata-sections -fno-strict-aliasing \
			$(ARCH) $(BUILD_FLAGS) $(G) \
		   -DPLUGIN_CREATOR=\"$(PLUGIN_CREATOR)\" -DPLUGIN_VERSION=\"$(PLUGIN_VERSION)\" -DPLUGIN_NAME=\"$(PLUGIN_NAME)\" \
		   -DUSE_SANGO_PLUGIN $(GAME_DEFINE) # -DUSE_DEFAULT_CTRPF

CFLAGS		+=	$(INCLUDE) -D__3DS__ $(DEFINES)

#-Wall -Wextra -Wdouble-promotion -Werror

# The plugin runs inside the memory of the game: no exceptions, no RTTI.
CXXFLAGS	:= $(CFLAGS) -fno-rtti -fno-exceptions -std=gnu++11

ASFLAGS		:= $(ARCH) $(G)
LDFLAGS		:= -T $(TOPDIR)/3gx.ld $(ARCH) -Os -Wl,$(WL)--gc-sections,--section-start,.text=0x07000100 #-specs=3dsx.specs

LIBS 		:=  $(BUILD_LIBS) -lm
LIBDIRS		:= 	$(CTRPFLIB) $(CTRULIB) $(PORTLIBS)

#---------------------------------------------------------------------------------
ifeq ($(strip $(PRODUCT)),)
#---------------------------------------------------------------------------------
# top level: one recursive make per product
#---------------------------------------------------------------------------------
.PHONY: all clean re $(PRODUCTS)

all: $(PRODUCTS)

$(PRODUCTS):
	@$(MAKE) --no-print-directory PRODUCT=$@

clean:
	@echo clean ...
	@rm -fr $(foreach p,$(PRODUCTS),release-$(p) debug-$(p)) release debug *.elf *.3gx

re: clean all

# `make run-<product>` builds one product, installs it as the plugin the
# emulator loads (always sango_plugin.3gx, so products never pile up) and
# launches the game. `make relink` is `make run-overlay`.
.PHONY: relink $(addprefix run-,$(PRODUCTS))

relink: run-overlay

$(addprefix run-,$(PRODUCTS)): run-%:
	# @test -n "$(DEST)" -a -n "$(EMULATOR)" -a -n "$(GAME_PATH)" || \
	#	{ echo "Set SDMC, EMULATOR and the game path in config.mk (see config.mk.example)."; exit 1; }
	@rm -f *.elf *.3gx
	@$(MAKE) --no-print-directory $*
	@mkdir -p "$(DEST)"
	@cp $($*_TARGET)-release.3gx "$(DEST)/sango_plugin.3gx"
	@$(EMULATOR) $(GAME_PATH)

#---------------------------------------------------------------------------------
else ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------
# product level: resolve the product's sources, then build in its own directory
#---------------------------------------------------------------------------------
TARGET		:=	$($(PRODUCT)_TARGET)
SOURCES		:=	$($(PRODUCT)_SOURCES)
INCLUDES	:=	$($(PRODUCT)_INCLUDES)
PSF			:=	$($(PRODUCT)_PSF)

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
					$(foreach dir,$(DATA),$(CURDIR)/$(dir))

export DEPSDIR	:=	$(CURDIR)/$(BUILD)

CFILES			:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
CCFILES         := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cc)))
SFILES			:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))

export LD 		:= 	$(CXX)
export OFILES	:=	$(CPPFILES:.cpp=.o)  $(CCFILES:.cc=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)

export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
					$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
					-I$(CURDIR)/$(BUILD)

export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L $(dir)/lib)

export PSF

.PHONY: all release-$(PRODUCT) debug-$(PRODUCT)

#---------------------------------------------------------------------------------
all: $(TARGET)-release.3gx

release-$(PRODUCT) debug-$(PRODUCT):
	@[ -d $@ ] || mkdir -p $@

$(TARGET)-release.3gx : release-$(PRODUCT)
	@$(MAKE) BUILD=release-$(PRODUCT) OUTPUT=$(CURDIR)/$@ BUILD_LIBS="-lctrpf -lctru" WL=--strip-discarded,--strip-debug, \
	BUILD_CFLAGS="-DNDEBUG=1 -O2 -fomit-frame-pointer" DEPSDIR=$(CURDIR)/release-$(PRODUCT) \
	--no-print-directory -C release-$(PRODUCT) -f $(CURDIR)/Makefile

$(TARGET)-debug.3gx : debug-$(PRODUCT)
	@$(MAKE) BUILD=debug-$(PRODUCT) OUTPUT=$(CURDIR)/$@ BUILD_LIBS="-lctrpfd -lctrud" BUILD_CFLAGS="-DDEBUG=1 -Og" G=-g \
	DEPSDIR=$(CURDIR)/debug-$(PRODUCT) --no-print-directory -C debug-$(PRODUCT) -f $(CURDIR)/Makefile

#---------------------------------------------------------------------------------
else
#---------------------------------------------------------------------------------
# build directory level: compile and link
#---------------------------------------------------------------------------------

DEPENDS	:=	$(OFILES:.o=.d)


$(OUTPUT) : $(basename $(OUTPUT)).elf
$(basename $(OUTPUT)).elf : $(OFILES)
#---------------------------------------------------------------------------------
# you need a rule like this for each extension you use as binary data
#---------------------------------------------------------------------------------
%.bin.o	:	%.bin
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	@$(bin2o)

#---------------------------------------------------------------------------------
%.o: %.cc
	$(SILENTMSG) $(notdir $<)
	$(ADD_COMPILE_COMMAND) add $(CXX) "$(_EXTRADEFS) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@" $<
	$(SILENTCMD)$(CXX) -MMD -MP -MF $(DEPSDIR)/$*.d $(_EXTRADEFS) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@ $(ERROR_FILTER)

#---------------------------------------------------------------------------------
%.3gx: %.elf
	@echo creating $(notdir $@)
	@3gxtool -s $^ $(TOPDIR)/$(PSF) $@

-include $(DEPENDS)

#---------------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------------
