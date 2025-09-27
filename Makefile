# Simple Makefile for GRID Emulation
# Usage:
#   make            # release build
#   make DEBUG=1    # debug build
#   make run        # build + run
#   make clean

APP      := grid
BUILD    := build

CXX      := g++
CXXSTD   := -std=gnu++17
WARN     := -Wall -Wextra -Wpedantic
DEFS     := -DGRID_EMULATION

DEBUG    ?= 0
ifeq ($(DEBUG),1)
  OPT := -O0 -g
else
  OPT := -O2
endif            # release build

SDL2_CFLAGS ?= $(shell pkg-config --cflags sdl2 2>/dev/null)

SDL2_LIBS   ?= $(shell pkg-config --libs sdl2 2>/dev/null)

ifeq ($(SDL2_CFLAGS),)

SDL2_CFLAGS := $(shell sdl2-config --cflags 2>/dev/null)

SDL2_LIBS   := $(shell sdl2-config --libs 2>/dev/null)

endif

INCLUDES := -Iemulator -IGRID

SRCS := \
  $(shell ls emulator/*.cpp) \
  $(shell ls GRID/*.cpp)

OBJS := $(addprefix $(BUILD)/,$(SRCS:.cpp=.o))
BIN  := $(BUILD)/$(APP)

.PHONY: all run clean
all: $(BIN)

$(BIN): $(OBJS)
	@mkdir -p $(@D)
	$(CXX) $(OPT) $(OBJS) $(SDL2_LIBS) -o $@

$(BUILD)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXSTD) $(OPT) $(WARN) $(DEFS) $(INCLUDES) $(SDL2_CFLAGS) -c $< -o $@

run: $(BIN)
	$(BIN)

clean:
	rm -rf $(BUILD)
