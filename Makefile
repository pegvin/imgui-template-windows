CXX        := g++
CXX_FLAGS  := -std=c++17 -Wall -MMD -MP -Ithird_party/imgui/include/
LFLAGS     := -lkernel32 -luser32 -lgdi32 -lwinspool -lshell32 -lole32 -loleaut32 -luuid -lcomdlg32 -ladvapi32 -lopengl32 -lglu32
BUILD      := build/
BUILD_TYPE := Debug
BIN        := MyApp.exe

SOURCES := src/main.cpp src/init.cpp
SOURCES += $(addprefix third_party/imgui/,imgui_demo.cpp imgui_impl_opengl3.cpp imgui_tables.cpp imgui.cpp imgui_draw.cpp imgui_impl_win32.cpp imgui_widgets.cpp)

OBJECTS := $(SOURCES:.cpp=.o)
OBJECTS := $(patsubst %,$(BUILD)/%,$(OBJECTS))
DEPENDS := $(OBJECTS:.o=.d)

ifeq ($(BUILD_TYPE),Debug)
	CXX_FLAGS+=-O0 -g
else
	ifeq ($(BUILD_TYPE),Release)
		CXX_FLAGS+=-O3
	else
$(error Unknown Build Type "$(BUILD_TYPE)")
	endif
endif

-include $(DEPENDS)

all: $(BIN)

$(BUILD)/%.o: %.cpp
	@echo "CXX -" $<
	@mkdir -p "$(dir $@)"
	@$(CXX) $(CXX_FLAGS) -c $< -o $@

$(BIN): $(OBJECTS)
	@echo Linking $@
	@$(CXX) $(LFLAGS) $(OBJECTS) -o $@

.PHONY: run
.PHONY: clean

run: $(all)
	./$(BIN)

clean:
	$(RM) $(OBJECTS) $(DEPENDS) $(BIN)

