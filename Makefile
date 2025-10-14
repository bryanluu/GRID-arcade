# Simple Makefile for GRID Emulation
# Usage:
#   make             # release build
#   make DEBUG=1     # debug build (ASan, symbols, -DDEBUG)
#   make run
#   make run-debug
#   make clean

APP   := grid-emulation
BUILD := build

CXX      := g++
CXXSTD   := -std=gnu++17
WARN     := -Wall -Wextra -Wpedantic
DEFS     := -DGRID_EMULATION

# SDL flags
SDL2_CFLAGS ?= $(shell pkg-config --cflags sdl2 2>/dev/null)
SDL2_LIBS   ?= $(shell pkg-config --libs sdl2 2>/dev/null)
ifeq ($(SDL2_CFLAGS),)
  SDL2_CFLAGS := $(shell sdl2-config --cflags 2>/dev/null)
  SDL2_LIBS   := $(shell sdl2-config --libs 2>/dev/null)
endif
LDLIBS := $(SDL2_LIBS)

# Build mode
DEBUG ?= 0
CXXFLAGS := $(CXXSTD) $(WARN) $(DEFS) $(SDL2_CFLAGS)
LDFLAGS  :=

ifeq ($(DEBUG),1)
  CXXFLAGS += -g -O0 -fno-omit-frame-pointer -fasynchronous-unwind-tables -DDEBUG -fsanitize=address
  LDFLAGS  += -fsanitize=address
else
  CXXFLAGS += -O2
endif

# Sources
INCLUDES := -Iemulation -IGRID
SRCS := $(wildcard emulation/*.cpp) $(wildcard GRID/*.cpp)
# Exclude RGBMatrix32.cpp from build (SDLMatrix32 used instead)
SRCS := $(filter-out GRID/RGBMatrix32.cpp,$(SRCS))
SRCS := $(filter-out GRID/ArduinoInputProvider.cpp,$(SRCS))
SRCS := $(filter-out GRID/FlashIO.cpp,$(SRCS))
OBJS := $(addprefix $(BUILD)/,$(SRCS:.cpp=.o))
BIN  := $(BUILD)/$(APP)

.PHONY: all run debug run-debug clean

all: $(BIN)

$(BIN): $(OBJS)
	@mkdir -p $(@D)
	$(CXX) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $@

$(BUILD)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Convenience targets
debug:
	$(MAKE) DEBUG=1

run: $(BIN)
	$(BIN)

clean:
	rm -rf $(BUILD)
