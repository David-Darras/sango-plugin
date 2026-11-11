# Builds and runs the tests on a computer: make -C tests
#
# The tests use the g++ of the computer, not devkitPro. They compile only the
# pure files of the library: the files without game structures.

ROOT     := $(abspath ..)
CXX      ?= g++
CXXFLAGS := -std=gnu++11 -Wall -Wextra -Wno-unused-parameter -O1 -DGAME_ORAS \
            -DSANGO_ROOT='"$(ROOT)"' \
            -I$(ROOT)/lib/include -I$(ROOT)/lib/src -I$(ROOT)/tests/host

# The pure files of the library.
LIB_SOURCES := $(ROOT)/lib/src/core/parse_float.cc \
               $(ROOT)/lib/src/core/utf8.cc
TEST_SOURCES := $(wildcard $(ROOT)/tests/*.cc)

# The program goes to debug-tests/: .gitignore ignores the debug-* folders.
OUT := $(ROOT)/debug-tests/run_tests

.PHONY: all clean
all: $(OUT)
	$(OUT)

$(OUT): $(LIB_SOURCES) $(TEST_SOURCES) $(wildcard $(ROOT)/tests/*.h)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -o $@ $(TEST_SOURCES) $(LIB_SOURCES)

clean:
	rm -rf $(ROOT)/debug-tests
