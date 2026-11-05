#---------------------------------------------------------------------------------
# Sango Plugin - build file
#
# Usage (from the devkitPro shell):
#   make                  builds all the products of the selected game
#   make overlay          builds one product (the name of its folder: overlay,
#                         kaizo, maker, myhack...)
#   make GAME=XY          builds for Pokemon X instead of Alpha Sapphire
#   make run-overlay      builds one product, copies it to the SD card of the
#                         emulator and starts the game (needs config.mk)
#   make release-pack     builds the Alpha Sapphire plugin and Sango Maker,
#                         and makes dist/Sango-Plugin-<version>.zip
#   make release          makes the zip, then publishes it on GitHub (needs gh)
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

PLUGIN_VERSION  := 7.5.0
PLUGIN_CREATOR  := ZettaD
PLUGIN_NAME     := Sango

# The links of the packs that the release README and notes show. Set them
# in config.mk.
POKEMON_MODELS_URL   ?= https://drive.google.com/file/d/1YgFf7eNm3bpnUxJ7xFQod6HtvWV6rjDw/view?usp=sharing
OVERWORLD_MODELS_URL ?= https://drive.google.com/file/d/1WghmLRZqxeH2gd0CmWTEP1VDFywphScH/view?usp=sharing
MUSIC_URL            ?= https://drive.google.com/file/d/16e-uJYl0zZbzPWCIFwjPsiPHwnlXFXBv/view?usp=sharing
TRAINER_MODELS_URL   ?= https://drive.google.com/file/d/1ph_P6EI7QYiVbTJeHZSUxzml-wrt1lsF/view?usp=sharing
GRASS_URL            ?= https://drive.google.com/file/d/1e54uM764ne0L0SSxjbT6bAY6Uq5h5Hfl/view?usp=sharing
PYTHON     ?= $(shell command -v python3 2>/dev/null || command -v python)

# The optimization of the release plugin. The plugin, its data and its heap
# share the memory of the .plgInfo (MemorySize): a smaller plugin leaves a
# bigger heap. -Os makes the smallest code. LTO=1 (the default) also removes
# the code and the data that no function uses, in all the files. `make LTO=0`
# builds without it, for example to compare the two.
LTO        ?= 1
LTO_FLAGS  := $(if $(filter 1,$(LTO)),-flto=auto)

#---------------------------------------------------------------------------------
# The target game.
#   ORAS: Pokemon Alpha Sapphire v1.4 (title id 000400000011C500), the main game.
#   XY:   Pokemon X v1.5 (title id 0004000000055D00), experimental, overlay only.
#---------------------------------------------------------------------------------
#---------------------------------------------------------------------------------
# The products. Each folder of the project with a .plgInfo file and a src/
# folder is a product. Its name is the name of the folder (`make myhack`).
# Its plugin file has the name of the .plgInfo file (myhack.plgInfo gives
# myhack-release.3gx). tools/new_hack.py makes a new product.
#---------------------------------------------------------------------------------
FOUND_PRODUCTS := $(foreach info,$(wildcard */*.plgInfo),\
	$(if $(wildcard $(dir $(info))src),$(patsubst %/,%,$(dir $(info)))))
# The overlay comes first.
FOUND_PRODUCTS := $(filter overlay,$(FOUND_PRODUCTS)) \
	$(sort $(filter-out overlay,$(FOUND_PRODUCTS)))

GAME      ?= ORAS
ifeq ($(GAME),XY)
GAME_DEFINE   := -DGAME_XY
GAME_PRODUCTS := overlay
TITLE_ID      := 0004000000055D00
GAME_PATH     ?= $(XY_GAME_PATH)
else
GAME_DEFINE   := -DGAME_ORAS
GAME_PRODUCTS := $(FOUND_PRODUCTS)
TITLE_ID      := 000400000011C500
GAME_PATH     ?= $(ORAS_GAME_PATH)
endif

# The plugin folder of the game on the SD card.
ifneq ($(strip $(SDMC)),)
DEST      ?= $(SDMC)/luma/plugins/$(TITLE_ID)
endif

#---------------------------------------------------------------------------------
# One library, several plugins built on it. `make` builds them all,
# `make overlay` / `make kaizo` just one; each gets its own build directory.
#
# To add your own ROM hack, type `python tools/new_hack.py myhack`.
# See docs/tutorials/01-create-your-rom-hack.md.
#---------------------------------------------------------------------------------
PRODUCTS	:=	$(GAME_PRODUCTS)

LIB_SOURCES	:=	lib/src \
				lib/src/boot \
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

# The settings of each product:
#   overlay/  sango_plugin.3gx: every page of the library under one menu
#   kaizo/    sango_kaizo.3gx: the ROM hack
#   maker/    sango_maker.3gx: Sango Maker, a ROM hack without code
# The data folder of a product on the SD card (see lib/include/core/data_folder.h):
# sdmc:/sango for the overlay, sdmc:/sango/<product> for the others.
define PRODUCT_SETTINGS
$(1)_PSF		:=	$(firstword $(wildcard $(1)/*.plgInfo))
$(1)_TARGET		:=	$$(basename $$(notdir $$($(1)_PSF)))
$(1)_SOURCES	:=	$(LIB_SOURCES) $(1)/src
$(1)_INCLUDES	:=	$(LIB_INCLUDES) $(1)/include
$(1)_DATA_FOLDER	:=	$(if $(filter overlay,$(1)),sango,sango/$(1))
endef
$(foreach product,$(FOUND_PRODUCTS),$(eval $(call PRODUCT_SETTINGS,$(product))))

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------
ARCH	:=	-march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft

CFLAGS	:=	-mword-relocations \
 			-ffunction-sections -fdata-sections -fno-strict-aliasing \
			$(ARCH) $(BUILD_CFLAGS) $(G) \
		   -DPLUGIN_CREATOR=\"$(PLUGIN_CREATOR)\" -DPLUGIN_VERSION=\"$(PLUGIN_VERSION)\" -DPLUGIN_NAME=\"$(PLUGIN_NAME)\" \
		   -DUSE_SANGO_PLUGIN $(GAME_DEFINE) $(PRODUCT_DEFINES)

CFLAGS		+=	$(INCLUDE) -D__3DS__ $(DEFINES)

#-Wall -Wextra -Wdouble-promotion -Werror

# The plugin runs inside the memory of the game: no exceptions, no RTTI.
# -fno-threadsafe-statics: a static of a function has no guard, so no mutex
# and no abort() (see core/singleton.h). -fcheck-new: the code checks the
# result of new (core/allocation.cc gives null when the heap is full).
CXX_ONLY	:= -fno-rtti -fno-exceptions -fno-threadsafe-statics -fcheck-new \
		   -std=gnu++11
CXXFLAGS	:= $(CFLAGS) $(CXX_ONLY)

ASFLAGS		:= $(ARCH) $(G)
# With LTO, the link compiles the code: it needs the same options.
LDFLAGS		:= -T $(TOPDIR)/3gx.ld $(ARCH) $(BUILD_CFLAGS) $(CXX_ONLY) -Wl,$(WL)--gc-sections,--section-start,.text=0x07000100 #-specs=3dsx.specs

LIBS 		:=  $(BUILD_LIBS) -lm
LIBDIRS		:= 	$(CTRULIB) $(PORTLIBS)

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
	@rm -fr $(foreach p,$(PRODUCTS),release-$(p) debug-$(p)) release debug dist *.elf *.3gx

# `make release-pack` builds the overlay and Sango Maker for Alpha Sapphire and
# puts them in dist/Sango-Plugin-<version>.zip with a README.txt, the example
# files of maker/data and the web editor of tools/maker_editor.
# `make release` then publishes the zip on GitHub (`make release DRAFT=1`
# makes a draft, `make release YES=1` does not ask first).
.PHONY: release-pack release

release-pack:
	@$(MAKE) --no-print-directory GAME=ORAS overlay maker
	@$(PYTHON) tools/release.py pack --version $(PLUGIN_VERSION) \
		--plugin $(overlay_TARGET)-release.3gx \
		--maker-plugin $(maker_TARGET)-release.3gx \
		--pokemon-models-url "$(POKEMON_MODELS_URL)" \
		--overworld-models-url "$(OVERWORLD_MODELS_URL)" \
		--music-url "$(MUSIC_URL)" \
		--trainer-models-url "$(TRAINER_MODELS_URL)" \
		--grass-url "$(GRASS_URL)"

release: release-pack
	@$(PYTHON) tools/release.py publish --version $(PLUGIN_VERSION) \
		$(if $(DRAFT),--draft) $(if $(YES),--yes)

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
export PRODUCT_DEFINES	:=	-DPRODUCT_DATA_FOLDER=\"$($(PRODUCT)_DATA_FOLDER)\"

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
	@$(MAKE) BUILD=release-$(PRODUCT) OUTPUT=$(CURDIR)/$@ BUILD_LIBS="-lctru" WL=--strip-discarded,--strip-debug, \
	BUILD_CFLAGS="-DNDEBUG=1 -Os -fomit-frame-pointer $(LTO_FLAGS)" DEPSDIR=$(CURDIR)/release-$(PRODUCT) \
	--no-print-directory -C release-$(PRODUCT) -f $(CURDIR)/Makefile

$(TARGET)-debug.3gx : debug-$(PRODUCT)
	@$(MAKE) BUILD=debug-$(PRODUCT) OUTPUT=$(CURDIR)/$@ BUILD_LIBS="-lctrud" BUILD_CFLAGS="-DDEBUG=1 -Og" G=-g \
	DEPSDIR=$(CURDIR)/debug-$(PRODUCT) --no-print-directory -C debug-$(PRODUCT) -f $(CURDIR)/Makefile

#---------------------------------------------------------------------------------
else
#---------------------------------------------------------------------------------
# build directory level: compile and link
#---------------------------------------------------------------------------------

DEPENDS	:=	$(OFILES:.o=.d)

# The first rule of the file is the default goal: keep it the plugin, not the
# rules of the macro files below.
.DEFAULT_GOAL := $(OUTPUT)

#---------------------------------------------------------------------------------
# The macros of the command line (-D...). The files of the build directory do not
# depend on the Makefile, so a change of a value did not rebuild anything. Each
# group of macros has a small file with its values: the make rule writes the file
# only when a value changed, and the object files depend on it.
#   game.flag    the target game (GAME_ORAS or GAME_XY): all the files
#   plugin.flag  the name, the version, the creator and the data folder: the
#                files that use them
#---------------------------------------------------------------------------------
GAME_VALUES   := $(GAME_DEFINE)
PLUGIN_VALUES := $(PLUGIN_NAME) $(PLUGIN_VERSION) $(PLUGIN_CREATOR) $(PRODUCT_DEFINES)

.PHONY: FORCE
FORCE:

game.flag: FORCE
	@echo '$(GAME_VALUES)' | cmp -s - $@ || echo '$(GAME_VALUES)' > $@

plugin.flag: FORCE
	@echo '$(PLUGIN_VALUES)' | cmp -s - $@ || echo '$(PLUGIN_VALUES)' > $@

# The source files that use the macros of the plugin.
PLUGIN_USERS := $(patsubst %.cc,%.o,$(notdir $(shell grep -lE \
	'PLUGIN_(NAME|VERSION|CREATOR)|SANGO_DATA_' $(wildcard $(addsuffix /*.cc,$(VPATH))) \
	2>/dev/null)))

$(OFILES): game.flag
$(PLUGIN_USERS): plugin.flag


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
