#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

export TOPDIR ?= $(CURDIR)
include $(DEVKITARM)/3ds_rules

PLUGIN_VERSION  := 5.0.0
PLUGIN_CREATOR  := ZettaD

# Target game: ORAS (Alpha Sapphire, 000400000011C500) or XY (X, 0004000000055D00).
GAME      ?= XY
ifeq ($(GAME),XY)
GAME_DEFINE := -DGAME_XY
TITLE_ID    := 0004000000055D00
GAME_PATH := "C:/Users/David/Desktop/ctr/cia/kujira.cci"
PLUGIN_NAME   := Kujira
else
GAME_DEFINE := -DGAME_ORAS
TITLE_ID    := 000400000011C500
GAME_PATH := "C:/Users/David/Desktop/ctr/cia/sango.3ds"
PLUGIN_NAME  := Sango
endif
DEST      := C:/Users/David/AppData/Roaming/Azahar/sdmc/luma/plugins/$(TITLE_ID)
EMULATOR  := "C:/Program Files/Azahar/azahar.exe"

CTRPFLIB	?=	$(DEVKITPRO)/libctrpf

#---------------------------------------------------------------------------------
# One library, several plugins built on it. `make` builds them all,
# `make overlay` / `make kaizo` just one; each gets its own build directory.
#---------------------------------------------------------------------------------
PRODUCTS	:=	overlay kaizo undertow

LIB_SOURCES	:=	lib/src \
				lib/src/core \
				lib/src/system \
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
				lib/src \
				../Library/include

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
		   -DUSE_SANGO_PLUGIN $(GAME_DEFINE) -DUSE_DEFAULT_CTRPF

CFLAGS		+=	$(INCLUDE) -D__3DS__ $(DEFINES)

#-Wall -Wextra -Wdouble-promotion -Werror

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
