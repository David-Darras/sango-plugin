#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

export TOPDIR ?= $(CURDIR)
include $(DEVKITARM)/3ds_rules

PLUGIN_VERSION  := 2.0.0
PLUGIN_CREATOR  := ZettaD

DEST      := C:/Users/David/AppData/Roaming/Azahar/sdmc/luma/plugins/000400000011C500
EMULATOR  := "C:/Program Files/Azahar/azahar.exe"
GAME_PATH := "C:/Users/David/Desktop/ctr/cia/sango.3ds"

CTRPFLIB	?=	$(DEVKITPRO)/libctrpf

TARGET		:= 	$(notdir $(CURDIR))
INCLUDES	:= 	include \
				../Library/include

SOURCES 	:= 	src \
				src/battle \
				src/core \
				src/data \
				src/hack \
				src/layout \
				src/menu \
				src/overworld \
				src/savedata \
				src/system

PSF 		:= 	$(notdir $(TOPDIR)).plgInfo

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------
ARCH	:=	-march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft

CFLAGS	:=	-mword-relocations \
 			-ffunction-sections -fdata-sections -fno-strict-aliasing \
			$(ARCH) $(BUILD_FLAGS) $(G) \
		   -DPLUGIN_CREATOR=\"$(PLUGIN_CREATOR)\" -DPLUGIN_VERSION=\"$(PLUGIN_VERSION)\"

CFLAGS		+=	$(INCLUDE) -D__3DS__ $(DEFINES)

#-Wall -Wextra -Wdouble-promotion -Werror

CXXFLAGS	:= $(CFLAGS) -fno-rtti -fno-exceptions -std=gnu++11

ASFLAGS		:= $(ARCH) $(G)
LDFLAGS		:= -T $(TOPDIR)/3gx.ld $(ARCH) -Os -Wl,$(WL)--gc-sections,--section-start,.text=0x07000100 #-specs=3dsx.specs

LIBS 		:=  $(BUILD_LIBS) -lm
LIBDIRS		:= 	$(CTRPFLIB) $(CTRULIB) $(PORTLIBS)

#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------
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

.PHONY: $(BUILD) clean re relink all

#---------------------------------------------------------------------------------
all: $(TARGET)-release.3gx

release:
	@[ -d $@ ] || mkdir -p $@

debug:
	@[ -d $@ ] || mkdir -p $@

$(TARGET)-release.3gx : release
	@$(MAKE) BUILD=release OUTPUT=$(CURDIR)/$@ BUILD_LIBS="-lctrpf -lctru" WL=--strip-discarded,--strip-debug, \
	BUILD_CFLAGS="-DNDEBUG=1 -O2 -fomit-frame-pointer" DEPSDIR=$(CURDIR)/release \
	--no-print-directory -C release	-f $(CURDIR)/Makefile

$(TARGET)-debug.3gx : debug
	@$(MAKE) BUILD=debug OUTPUT=$(CURDIR)/$@ BUILD_LIBS="-lctrpfd -lctrud" BUILD_CFLAGS="-DDEBUG=1 -Og" G=-g \
	DEPSDIR=$(CURDIR)/debug --no-print-directory -C debug -f $(CURDIR)/Makefile

#---------------------------------------------------------------------------------
clean:
	@echo clean ...
	@rm -fr release debug *.elf *.3gx

re: clean all

relink:
	@rm -f *.elf *.3gx
	@$(MAKE)
	@cp $(TARGET)-release.3gx "$(DEST)/$(TARGET).3gx"
	@$(EMULATOR) $(GAME_PATH)

#---------------------------------------------------------------------------------

else

#---------------------------------------------------------------------------------
# main targets
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
