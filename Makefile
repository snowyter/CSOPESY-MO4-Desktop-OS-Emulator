# Makefile -- builds the CSOPESY Desktop OS Emulator (MO4)
# ---------------------------------------------------------------------------
# Why a Makefile instead of one long g++ command (like MO3)?
#   Dear ImGui is ~70,000 lines. Recompiling it on every F5 would take ~30 s.
#   make keeps each compiled file in build/ and only recompiles what changed,
#   so after the first build, F5 only rebuilds the files you edited.
#
# Usage:   mingw32-make          (VS Code's F5 does this for you)
#          mingw32-make clean    (delete build output, forces a full rebuild)
# ---------------------------------------------------------------------------

CXX      ?= g++
CXXFLAGS := -std=c++17 -O2 -g -Wall -MMD -MP \
            -Isrc -Iexternal/imgui -Iexternal/imgui/backends -Iexternal/stb

# Our own source files + the parts of Dear ImGui we use.
SRC_FILES   := $(wildcard src/*.cpp)
IMGUI_FILES := imgui.cpp imgui_draw.cpp imgui_tables.cpp imgui_widgets.cpp \
               imgui_impl_glfw.cpp imgui_impl_opengl3.cpp

OBJS := $(patsubst src/%.cpp,build/%.o,$(SRC_FILES)) \
        $(patsubst %.cpp,build/%.o,$(IMGUI_FILES))

# ---- Platform-specific settings -------------------------------------------
ifeq ($(OS),Windows_NT)
    TARGET   := csopesy.exe
    CXXFLAGS += -Iexternal/glfw/include
    # -static: no DLLs needed next to the .exe
    # -mwindows: a GUI program, so no extra black console window appears
    LDFLAGS  := -static -mwindows -Lexternal/glfw/lib-mingw-w64
    LDLIBS   := -lglfw3 -lopengl32 -lgdi32
    RM_BUILD := if exist build\*.o del /Q build\*.o build\*.d & if exist $(TARGET) del /Q $(TARGET)
else
    # Linux/macOS (used for testing): needs the system GLFW package
    TARGET   := csopesy
    LDFLAGS  :=
    LDLIBS   := -lglfw -lGL -lpthread
    RM_BUILD := rm -f build/*.o build/*.d $(TARGET)
endif

vpath %.cpp src external/imgui external/imgui/backends

# ---- Rules -------------------------------------------------------------------
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS) $(LDLIBS)

build/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	$(RM_BUILD)

.PHONY: all clean

# Header dependency files written by -MMD (rebuild a .cpp when its .h changes)
-include $(OBJS:.o=.d)
