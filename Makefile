#---------------------------------------------------------------------------------
# Project Configuration
#---------------------------------------------------------------------------------
.SUFFIXES:

# Check if DevkitARM is defined
ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

# Project Paths and Metadata
CITRA_PATH      := C:/Users/David/AppData/Roaming/Citra/cheats/000400000011C500.txt
ADDRESS_SOURCE  := 0x00122938
ADDRESS_TARGET  := 0x00696000

PLUGIN_VERSION  := 1.0.0
PLUGIN_CREATOR  := ZettaD

TOPDIR          ?= $(CURDIR)
include $(DEVKITARM)/3ds_rules

# Directories and Files
TARGET          := $(notdir $(CURDIR))
BUILD           := bin
INCLUDES        := include
SOURCES         := src src/menu src/system src/savedata src/overworld

# Architecture and Flags
ARCH            := -march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft

CFLAGS          := $(ARCH) -Os -fomit-frame-pointer -mword-relocations \
                   -ffunction-sections -fno-strict-aliasing \
                   -DADDRESS_SOURCE=$(ADDRESS_SOURCE) -DADDRESS_TARGET=$(ADDRESS_TARGET) \
                   -DPLUGIN_CREATOR=\"$(PLUGIN_CREATOR)\" -DPLUGIN_VERSION=\"$(PLUGIN_VERSION)\"

CFLAGS          += $(INCLUDE) -D__3DS__

CXXFLAGS        := $(CFLAGS) -fno-rtti -fno-exceptions -std=gnu++11

ASFLAGS         := $(ARCH) -DADDRESS_SOURCE=$(ADDRESS_SOURCE) -DADDRESS_TARGET=$(ADDRESS_TARGET)
LDFLAGS         := -T $(TOPDIR)/arcc.ld $(ARCH) -Os -nostartfiles -fno-lto -Wl,--gc-sections,--strip-discarded,--strip-debug

#---------------------------------------------------------------------------------
# Build Logic (Recursive Makefile)
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))

export OUTPUT   := $(CURDIR)/$(TARGET)
export TOPDIR   := $(CURDIR)
export VPATH    := $(foreach dir,$(SOURCES),$(CURDIR)/$(dir))
export DEPSDIR  := $(CURDIR)/$(BUILD)

# Locate Source Files
CFILES          := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES        := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
CCFILES         := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cc)))
SFILES          := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))

export LD       := $(CXX)
export OFILES   := $(CPPFILES:.cpp=.o) $(CCFILES:.cc=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)
export INCLUDE  := $(foreach dir,$(INCLUDES),-I $(CURDIR)/$(dir)) \
                   -I $(CURDIR)/$(BUILD)

export LIBPATHS := $(foreach dir,$(LIBDIRS),-L $(dir)/lib)

.PHONY: $(BUILD) all clean re

#---------------------------------------------------------------------------------
# Main Rules
#---------------------------------------------------------------------------------
all: $(BUILD)

$(BUILD):
	@echo "Cleaning previous build..."
	@rm -fr $(BUILD) $(OUTPUT).txt $(OUTPUT).elf
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile
	@echo "Copying $(notdir $(OUTPUT).txt) to Citra cheats folder..."
	@cp "$(OUTPUT).txt" "$(CITRA_PATH)"
	@echo "Done!"

clean:
	@echo "Cleaning project..."
	@rm -fr $(BUILD) $(OUTPUT).txt $(OUTPUT).elf

re: all

#---------------------------------------------------------------------------------
else

# Sub-process compilation rules
DEPENDS := $(OFILES:.o=.d)

$(OUTPUT).txt : $(OFILES)

# Rules for binary data
%.bin.o : %.bin
	@echo $(notdir $<)
	@$(bin2o)

# Rules for C++ files
%.o: %.cc
	$(SILENTMSG) $(notdir $<)
	$(ADD_COMPILE_COMMAND) add $(CXX) "$(_EXTRADEFS) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@" $<
	$(SILENTCMD)$(CXX) -MMD -MP -MF $(DEPSDIR)/$*.d $(_EXTRADEFS) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@ $(ERROR_FILTER)

# Final conversion to text format
.PRECIOUS: %.elf
%.txt: %.elf
	@echo "Creating $(notdir $@)..."
	@pwd
	@elf2arcc $< $(ADDRESS_SOURCE) $(ADDRESS_TARGET) > $@

-include $(DEPENDS)

endif