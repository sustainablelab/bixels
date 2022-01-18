EXE = bob
all: $(EXE)
	@echo Build complete for $(ECHO_MESSAGE)

# move these to top of the file to make them default target
file: _file.exe
calloc: _calloc.exe
array: _array.exe

IMGUI_DIR = ./imgui
SOURCES = main.cpp

OBJS = $(addsuffix .o, $(basename $(notdir $(SOURCES))))
SOURCES += $(IMGUI_DIR)/imgui.cpp $(IMGUI_DIR)/imgui_demo.cpp $(IMGUI_DIR)/imgui_draw.cpp $(IMGUI_DIR)/imgui_tables.cpp $(IMGUI_DIR)/imgui_widgets.cpp
SOURCES += $(IMGUI_DIR)/backends/imgui_impl_sdl.cpp $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp

CXXFLAGS = -g -Wall -Wformat -O3
CXXFLAGS += -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends
LIBS =

ifeq ($(OS), Windows_NT)
	ECHO_MESSAGE = "MinGW"
	LIBS += -lgdi32 -lopengl32 -limm32
	LIBS += `pkg-config --static --libs sdl2`
	CXXFLAGS += `pkg-config --cflags sdl2`
	# CFLAGS = $(CXXFLAGS)
endif

##---------------------------------------------------------------------
## BUILD RULES
##---------------------------------------------------------------------

%.o:%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:$(IMGUI_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:$(IMGUI_DIR)/backends/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(EXE): $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS)


.PHONY: _file.exe
_file.exe: _file.c
	$(CXX) -o $@ $^  -g -O0 -Wall
	$(CXX) -S $^     -g -O0 -Wall

.PHONY: _calloc.exe
_calloc.exe: _calloc.c
	$(CXX) -o $@ $^  -g -O0 -Wall
	$(CXX) -S $^     -g -O0 -Wall

.PHONY: _array.exe
_array.exe: _array.c
	$(CXX) -o $@ $^ -g -O0 -Wall
	$(CXX) -S $^    -g -O0 -Wall

.PHONY: what-compiler
what-compiler:
	@echo $(CXX)

.PHONY: clean
clean:
	rm -f $(EXE) $(OBJS)
	rm -f libs.txt
.PHONY: print-libs
print-libs: main.cpp
	$(CXX) $(CXXFLAGS) $< -M > libs.txt
.PHONY: tags
tags: main.cpp
	ctags --c-kinds=+l --exclude=Makefile -R .
.PHONY: lib-tags
lib-tags: main.cpp
	$(CXX) $(CXXFLAGS) $< -M > headers-windows.txt
	python.exe parse-lib-tags.py
	rm -f headers-windows.txt
	ctags -f lib-tags --c-kinds=+p -L headers-posix.txt
	rm -f headers-posix.txt


