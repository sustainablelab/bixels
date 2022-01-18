EXE = bob
all: $(EXE)
	@echo Build complete for $(ECHO_MESSAGE)

IMGUI_DIR = ./imgui
SOURCES = main.cpp

OBJS = $(addsuffix .o, $(basename $(notdir $(SOURCES))))
SOURCES += $(IMGUI_DIR)/imgui.cpp $(IMGUI_DIR)/imgui_demo.cpp $(IMGUI_DIR)/imgui_draw.cpp $(IMGUI_DIR)/imgui_tables.cpp $(IMGUI_DIR)/imgui_widgets.cpp
SOURCES += $(IMGUI_DIR)/backends/imgui_impl_glfw.cpp $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp

CXXFLAGS = -g -Wall -Wformat -O3
CXXFLAGS += -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends
LIBS =

ifeq ($(OS), Windows_NT)
	ECHO_MESSAGE = "MinGW"
	LIBS += -mwindows
	LIBS += -lglfw3 -lgdi32 -lopengl32 -limm32 -lglew32 -lglu32
	CXXFLAGS += `pkg-config --cflags glfw3`
	CXXFLAGS += -DBOB

	CFLAGS = $(CXXFLAGS)
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


