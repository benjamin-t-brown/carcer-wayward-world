EXE=CARCER
EXE_SUFFIX=

MAIN_ALL=main.cpp

CODE=\
db/Database.cpp \
db/loaders/LoadItemTemplates.cpp \
db/loaders/LoadSpecialEvents.cpp \
layers/Layer.cpp \
layers/LayerManager.cpp \
layers/ui/LayerContextMenuInventoryItem.cpp \
layers/ui/LayerContextMenuPickUpItem.cpp \
model/Player.cpp \
model/Character.cpp \
runner/SpecialEventRunner.cpp \
runner/ConditionEvaluator.cpp \
runner/StringEvaluator.cpp \
runner/EventRunnerHelpers.cpp \
state/DatabaseInterface.cpp \
state/StateManager.cpp \
state/StateManagerInterface.cpp \
state/LayerManagerInterface.cpp \
ui/UiElement.cpp \
ui/FontScale.cpp \
ui/elements/SpriteElement.cpp \
ui/elements/Quad.cpp \
ui/elements/TextLine.cpp \
ui/elements/TextParagraph.cpp \
ui/elements/buttons/ButtonClose.cpp \
ui/elements/buttons/ButtonModal.cpp \
ui/elements/buttons/ButtonScroll.cpp \
ui/elements/buttons/ButtonGroup.cpp \
ui/elements/SectionDropShadow.cpp \
ui/elements/SectionScrollable.cpp \
ui/elements/buttons/ButtonTextWrap.cpp \
ui/elements/buttons/ButtonWorldAction.cpp \
ui/elements/OutsetRectangle.cpp \
ui/components/BorderModalStandard.cpp \
ui/components/BorderModalSmall.cpp \
ui/components/BorderInGameWide.cpp \
ui/components/BorderInGameNarrow.cpp \
ui/components/ChCompactInfo.cpp \
ui/components/VerticalList.cpp \
ui/components/HorizontalList.cpp \
ui/layouts/InGameLayout.cpp \
ui/layouts/ModalStandard.cpp \
ui/layouts/ModalSmall.cpp \
ui/minipages/MinipageCharacterSheet.cpp \
ui/minipages/MinipageEvent.cpp \
ui/components/lists/ListInventory.cpp \
ui/components/lists/ListPickUp.cpp \
ui/components/lists/ListChCompactInfoVertical.cpp \
ui/components/lists/ListChCompactInfoHorizontal.cpp \
ui/popups/PopupInventoryItem.cpp \
 ui/pages/PageCharacter.cpp \
ui/pages/PageInventory.cpp \
ui/pages/PageMagicSetup.cpp \
ui/pages/PageTalkChoice.cpp \
ui/minipages/MinipagePickUp.cpp \
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

DEPENDS := $(patsubst %.cpp,%.d,$(CODE)) main.d
LIB_DEPENDS := $(patsubst %.cpp,%.d,$(LIB_CODE))

SDL2W_STAMP = $(SDL2W_DIR_NAME)/.source_ok
SDL2W_LIB = lib/sdl2w/libsdl2w.a

# Subset of Emscripten port flags: needed when compiling sdl2w to .o (static lib), not only at final link.
EMCC_SDL_PORTS=\
-s USE_SDL=2\
-s USE_SDL_IMAGE=2\
-s USE_SDL_MIXER=2\
-s SDL2_IMAGE_FORMATS='["png"]'\
-s USE_SDL_TTF=2\
-s USE_SDL_GFX=2

# Link-time (includes EMCC_SDL_PORTS).
EMCC_LIBS=\
$(EMCC_SDL_PORTS)\
-s ALLOW_MEMORY_GROWTH=1\
-s SAFE_HEAP=0\
-s ASSERTIONS=1\
-s INITIAL_MEMORY=326565888\
-s ENVIRONMENT=web\
-s DISABLE_EXCEPTION_CATCHING=0\
-lidbfs.js

EMCC_EXPORTED=\
-s EXPORTED_FUNCTIONS='[\
 "_main",\
 "_enableSound",\
 "_disableSound",\
 "_setVolume",\
 "_setKeyDown",\
 "_setKeyUp",\
 "_setKeyStatus",\
 "_sendEvent"\
]'\
-s EXPORTED_RUNTIME_METHODS=['ccall']

DEBUG = true
LIBS += -Llib/sdl2w -lsdl2w 
INCLUDES += -I.
CPP_STANDARD = -std=c++23
ifeq ($(TARGET),wasm)
    FLAGS = -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers $(CPP_STANDARD) -Oz
    LIBS += $(EMCC_LIBS) $(EMCC_EXPORTED) --preload-file assets
    EXE_SUFFIX = .js
    CXX = em++
else
    FLAGS = -O0 -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers $(CPP_STANDARD) -fno-omit-frame-pointer
    ifndef CXX
        CXX = g++
    endif
    ifeq ($(OS),Windows_NT)
        LIBS += -mconsole -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2_gfx
    else
        UNAME_S := $(shell uname -s)
        ifeq ($(UNAME_S),Darwin)
            LIBS += -L/opt/homebrew/lib -L/usr/local/lib -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2_gfx
            INCLUDES += -I/opt/homebrew/include -I/usr/local/include
        else
            LIBS += -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2_gfx
        endif
    endif
    ifeq ($(DEBUG),true)
        FLAGS += -g
    endif
endif

.PHONY: all sdl2w_wasm sdl2w-rebuild _copy_sdl2w_artifacts

all: sdl2w
	$(MAKE) $(EXE)

$(EXE): $(OBJECTS) $(LIB_OBJECTS) $(MAIN_ALL_OBJECT) $(SDL2W_LIB)
	$(CXX) $(FLAGS) $(INCLUDES) $(MAIN_ALL_OBJECT) $(OBJECTS) $(LIB_OBJECTS) -o $(EXE)$(EXE_SUFFIX) $(LIBS)

# Convenience alias: builds $(SDL2W_LIB) only when needed (see rule below).
sdl2w: $(SDL2W_LIB)

# Force a full sdl2w rebuild + copy (use after upgrading sdl2w or changing its build).
sdl2w_rebuild:
	$(MAKE) -C "$(SDL2W_DIR_NAME)/src" all
	@$(MAKE) _copy_sdl2w_artifacts

$(SDL2W_STAMP):
	@if [ ! -d $(SDL2W_DIR_NAME) ]; then \
		echo "Directory $(SDL2W_DIR_NAME) not found. Cloning $(SDL2W_REPO_URL) into $(SDL2W_DIR_NAME)..."; \
		git clone $(SDL2W_REPO_URL) $(SDL2W_DIR_NAME); \
	else \
		echo "Directory $(SDL2W_DIR_NAME) found. Assuming it is the sdl2w repository."; \
	fi
	@touch $@

$(SDL2W_LIB): $(SDL2W_STAMP)
	@echo "Building standard sdl2w library (running 'make all' in $(SDL2W_DIR_NAME)/src)...";
	$(MAKE) -C "$(SDL2W_DIR_NAME)/src" all
	@$(MAKE) _copy_sdl2w_artifacts

sdl2w_wasm: $(SDL2W_STAMP)
	@echo "Building sdl2w library for WebAssembly (running 'make wasm' in $(SDL2W_DIR_NAME)/src)...";
	$(MAKE) -C "$(SDL2W_DIR_NAME)/src" wasm
	@$(MAKE) _copy_sdl2w_artifacts

_copy_sdl2w_artifacts:
	@mkdir -p lib/sdl2w
	@echo "Copying sdl2w artifacts from $(SDL2W_BUILD_OUTPUT_DIR) to lib/sdl2w..."
	@cp $(SDL2W_BUILD_OUTPUT_DIR)/lib/libsdl2w.a lib/sdl2w
	@cp $(SDL2W_BUILD_OUTPUT_DIR)/include/*.h lib/sdl2w

compiler_args:
	@echo $(FLAGS) $(INCLUDES) $(OBJECTS) $(LIB_OBJECTS) $(LIBS)

object_files: $(OBJECTS) $(LIB_OBJECTS)

-include $(DEPENDS)
-include $(LIB_DEPENDS)

%.o: %.cpp
	$(CXX) $(FLAGS) $(INCLUDES) -MMD -MP -c $< -o $@

run:
	./$(EXE)

js: sdl2w_wasm
	$(MAKE) TARGET=wasm $(EXE)
	mv $(EXE).js ../web/
	mv $(EXE).data ../web/
	mv $(EXE).wasm ../web/

debug:
	gdb $(EXE)

clean:
	rm -f $(OBJECTS) $(LIB_OBJECTS) $(MAIN_ALL_OBJECT)
	rm -f $(DEPENDS) $(LIB_DEPENDS)
	rm -f $(EXE) $(EXE).exe
	rm -rf main.d
	rm -rf .build
	rm -rf lib/sdl2w
	rm -rf $(SDL2W_BUILD_OUTPUT_DIR)
	rm -rf $(SDL2W_DIR_NAME)/src/build/
	rm -rf ../dist/$(EXE).*
	rm -rf $(SDL2W_STAMP)
