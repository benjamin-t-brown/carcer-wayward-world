EXE=CARCER
EXE_SUFFIX=

MAIN_ALL=main.cpp

CODE=\
LibHTML.cpp \
db/Database.cpp \
db/spriteMappings.cpp \
db/loaders/LoadItemTemplates.cpp \
db/loaders/LoadSpecialEvents.cpp \
layers/Layer.cpp \
layers/LayerManager.cpp \
layers/ui/LayerContextMenuInventoryItem.cpp \
layers/ui/LayerContextMenuPickUpItem.cpp \
model/Player.cpp \
model/Character.cpp \
state/DatabaseInterface.cpp \
state/StateManager.cpp \
state/StateManagerInterface.cpp \
state/LayerManagerInterface.cpp \
ui/UiElement.cpp \
ui/elements/SpriteElement.cpp \
ui/elements/Quad.cpp \
ui/elements/TextLine.cpp \
ui/elements/TextParagraph.cpp \
ui/elements/ButtonClose.cpp \
ui/elements/ButtonModal.cpp \
ui/elements/ButtonScroll.cpp \
ui/elements/ButtonGroup.cpp \
ui/elements/SectionDropShadow.cpp \
ui/elements/SectionScrollable.cpp \
ui/elements/ButtonTextWrap.cpp \
ui/elements/ButtonWorldAction.cpp \
ui/elements/OutsetRectangle.cpp \
ui/components/BorderModalStandard.cpp \
ui/components/BorderModalSmall.cpp \
ui/components/BorderInGame.cpp \
ui/components/VerticalList.cpp \
ui/layouts/InGameLayout.cpp \
ui/layouts/ModalStandard.cpp \
ui/layouts/ModalSmall.cpp \
ui/lists/ListInventory.cpp \
ui/lists/ListPickUp.cpp \
ui/popups/PopupInventoryItem.cpp \
ui/pages/PageInventory.cpp \
ui/pages/PagePickUp.cpp \
ui/popups/PopupPickupItem.cpp \

LIB_CODE=\

TEST_CODE=\

TEST_CODE_UI=\

TARGET ?= native
BASE_BUILD_DIR = build

SDL2W_REPO_URL = https://github.com/benjamin-t-brown/sdl2w.git
SDL2W_DIR_NAME = ../sdl2w
SDL2W_BUILD_OUTPUT_DIR = $(SDL2W_DIR_NAME)/sdl2w

MAIN_ALL_OBJECT=$(MAIN_ALL:.cpp=.o)

OBJECTS=$(CODE:.cpp=.o)
LIB_OBJECTS=$(LIB_CODE:.cpp=.o)

DEPENDS := $(patsubst %.cpp,%.d,$(CODE))
LIB_DEPENDS := $(patsubst %.cpp,%.d,$(LIB_CODE))

EMCC_LIBS=\
-s USE_SDL=2\
-s USE_SDL_IMAGE=2\
-s USE_SDL_MIXER=2\
-s SDL2_IMAGE_FORMATS='["png"]'\
-s USE_SDL_TTF=2\
-s USE_SDL_GFX=2\
-s ALLOW_MEMORY_GROWTH=1\
-s SAFE_HEAP=0\
-s ASSERTIONS=1\
-s INITIAL_MEMORY=326565888\
-s ENVIRONMENT=web\
-s JS_MATH=0\
-s DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR\
-s NO_DISABLE_EXCEPTION_CATCHING\
-lidbfs.js\
-Llib/sdl2w -lsdl2w

EMCC_EXPORTED=\
-s EXPORTED_FUNCTIONS='[\
 "_main",\
 "_enableSound",\
 "_disableSound",\
 "_setVolume",\
 "_setKeyDown",\
 "_setKeyUp",\
 "_setKeyStatus"\
]'\
-s EXPORTED_RUNTIME_METHODS=['ccall']

TARGET ?= native
BASE_BUILD_DIR = build

DEBUG=true
LIBS=-Llib/sdl2w
INCLUDES= -I.
ifeq ($(TARGET),wasm)
    FLAGS = -Wall -std=c++20 -Oz -flto
    LIBS = $(EMCC_LIBS) --preload-file assets
		EXE_SUFFIX=.js
    CXX = em++
else
    FLAGS = -O0 -Wall -std=c++20 -I.
    CXX = g++
    ifeq ($(OS),Windows_NT)
        LIBS += -fuse-ld=lld -lsdl2w -mconsole -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2_gfx
    else
        UNAME_S := $(shell uname -s)
        ifeq ($(UNAME_S),Darwin)
            LIBS += -L/opt/homebrew/lib -L/usr/local/lib -lsdl2w -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2_gfx
            INCLUDES += -I/opt/homebrew/include -I/usr/local/include
        else
            LIBS += -lsdl2w -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2_gfx
        endif
    endif
    ifeq ($(DEBUG),true)
        FLAGS += -g
    endif
    ifeq ($(TARGET),miyoo)
        AS=arm-linux-gnueabihf-as
        CC=arm-linux-gnueabihf-gcc
        CXX=arm-linux-gnueabihf-g++
        LD=arm-linux-gnueabihf-ld
				INCLUDES += $$SDL_INCLUDE
				LIBS += $$SDL_LIB_PATHS -lsdl2w -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2_gfx
    endif
endif

# Updated .PHONY to include helper targets and remove the old $(SDL2W_BUILD_OUTPUT_DIR)
.PHONY: $(EXE) all object_files compiler_args sdl2w sdl2w_wasm _ensure_sdl2w_source _copy_sdl2w_artifacts

$(EXE): $(OBJECTS) $(LIB_OBJECTS) $(MAIN_ALL_OBJECT)
	$(CXX) $(FLAGS) $(INCLUDES) $(MAIN_ALL_OBJECT) $(OBJECTS) $(LIB_OBJECTS) -o $(EXE)$(EXE_SUFFIX) $(LIBS)

all: $(EXE) sdl2w

compiler_args:
	@echo $(FLAGS) $(INCLUDES) $(OBJECTS) $(LIB_OBJECTS) $(LIBS)

object_files: $(OBJECTS) $(LIB_OBJECTS)

sdl2w: _ensure_sdl2w_source
	@echo "Building standard sdl2w library (running 'make all' in $(SDL2W_DIR_NAME)/src)...";
	$(MAKE) -C "$(SDL2W_DIR_NAME)/src" -j8 all
	$(MAKE) _copy_sdl2w_artifacts

sdl2w_wasm: _ensure_sdl2w_source
	@echo "Building sdl2w library for WebAssembly (running 'make wasm' in $(SDL2W_DIR_NAME)/src)...";
	$(MAKE) -C "$(SDL2W_DIR_NAME)/src" -j8 wasm
	$(MAKE) _copy_sdl2w_artifacts

# Common target to ensure sdl2w source code is present
_ensure_sdl2w_source:
	@if [ ! -d $(SDL2W_DIR_NAME) ]; then \
		echo "Directory $(SDL2W_DIR_NAME) not found. Cloning $(SDL2W_REPO_URL) into $(SDL2W_DIR_NAME)..."; \
		git clone $(SDL2W_REPO_URL) $(SDL2W_DIR_NAME); \
	else \
		echo "Directory $(SDL2W_DIR_NAME) found. Assuming it is the sdl2w repository."; \
	fi

# Common target to copy built sdl2w artifacts
# This assumes the sub-make (all or wasm) places artifacts correctly in $(SDL2W_BUILD_OUTPUT_DIR)
_copy_sdl2w_artifacts:
	@mkdir -p lib/sdl2w
	@echo "Copying sdl2w artifacts from $(SDL2W_BUILD_OUTPUT_DIR) to lib/sdl2w..."
	@cp -v $(SDL2W_BUILD_OUTPUT_DIR)/lib/libsdl2w.a lib/sdl2w
	@cp -v $(SDL2W_BUILD_OUTPUT_DIR)/include/*.h lib/sdl2w

-include $(DEPENDS)
-include $(LIB_DEPENDS)

%.o: %.cpp
	$(CXX) $(FLAGS) $(INCLUDES) -MMD -MP -c $< -o $@

run:
	./$(EXE)

temp:
	mkdir -p .build

js: sdl2w_wasm
	make TARGET=wasm -j8
	mv $(EXE).js ../web/
	mv $(EXE).data ../web/
	mv $(EXE).wasm ../web/

clean:
	rm -f $(OBJECTS) $(LIB_OBJECTS) $(MAIN_ALL_OBJECT)
	rm -f $(DEPENDS) $(LIB_DEPENDS)
	rm -f $(EXE) $(EXE).exe
	rm -rf main.d
	rm -rf .build
	rm -rf dist
	rm -rf ../web/$(EXE).*

clean_sdl2w:
	rm -rf lib/sdl2w
	rm -rf $(SDL2W_BUILD_OUTPUT_DIR)
	rm -rf $(SDL2W_DIR_NAME)/src/build/
