EXE=CARCER
EXE_SUFFIX=

MAIN_ALL=main.cpp

# Implementation units (.cpp) sit next to their interface units (.cppm).
# Umbrella + macros.h live in modules/. BMI graph: modules/make/build-bmi.mk.
CODE=\
db/Database.cpp \
db/loaders/LoadItemTemplates.cpp \
db/loaders/LoadAbilityJson.cpp \
db/loaders/LoadAbilityTemplates.cpp \
db/loaders/LoadSpellTemplates.cpp \
db/loaders/LoadStatusEffectTemplates.cpp \
db/loaders/LoadCharacterTemplates.cpp \
db/loaders/LoadMapTemplates.cpp \
db/loaders/LoadMapGridTemplates.cpp \
db/loaders/LoadTilesetTemplates.cpp \
db/loaders/LoadSpecialEvents.cpp \
lib/Json.cpp \
lib/hiscore/hiscore.cpp \
layers/LayerManager.cpp \
game/combat/SpellRules.cpp \
game/map/ActiveMapOrchestrator.cpp \
game/map/Camera.cpp \
game/map/MapWalkability.cpp \
game/map/MapVision.cpp \
game/map/TileFields.cpp \
game/map/MapPersistence.cpp \
game/map/MapPathfinding.cpp \
game/map/MapPickup.cpp \
game/map/TileDistance.cpp \
game/map/TileTriggers.cpp \
game/combat/projectileHelpers.cpp \
game/combat/Damage.cpp \
game/diceHelpers.cpp \
game/combat/EnemyBehavior.cpp \
in3/SpecialEventRunner.cpp \
in3/ConditionEvaluator.cpp \
in3/StringEvaluator.cpp \
in3/EventRunnerHelpers.cpp \
state/DatabaseInterface.cpp \
state/ActionBus.cpp \
state/StateManager.cpp \
state/StateManagerInterface.cpp \
state/UiManager.cpp \
state/LayerManagerInterface.cpp \
state/WorldUpdater.cpp \
ui/UiElement.cpp \
ui/FontScale.cpp \
ui/helpers/worldActions.cpp \
ui/helpers/keyboardShortcuts.cpp \
ui/helpers/modalLayoutFit.cpp \
ui/KeyboardHeldScroll.cpp \
ui/components/ChCompactInfo.cpp \
actions/combat/combat.cpp \
game/combat/CombatRunner.cpp \

TARGET ?= native

.DEFAULT_GOAL := all

SDL2W_REPO_URL = https://github.com/benjamin-t-brown/sdl2w.git
SDL2W_DIR_NAME = ../../sdl2w
SDL2W_REF = experiment/cpp-modules
SDL2W_BUILD_OUTPUT_DIR = $(SDL2W_DIR_NAME)/sdl2w
SDL2W_MODULES_LIB = lib/sdl2w/libsdl2w_modules.a

MAIN_ALL_OBJECT=$(MAIN_ALL:.cpp=.o)
OBJECTS=$(CODE:.cpp=.o)
DEPENDS := $(patsubst %.cpp,%.d,$(CODE)) main.d
LIBCARCER = lib/libcarcer.a
CARCER_BMI_STAMP = gcm.cache/.carcer-ready
SDL2W_STAMP = $(SDL2W_DIR_NAME)/.source_ok

# Copied layout from copy-sdl2w-artifacts.sh (archives next to modules/, not in lib/).
# Set paths before include so use.mk's ?= defaults do not pick the in-tree build/ path.
SDL2W_LIB = $(abspath lib/sdl2w/libsdl2w_modules.a)
BMIN_LIB = $(abspath lib/sdl2w/libbmin_modules.a)
SDL2W_LIBDIR = $(abspath lib/sdl2w)
BMIN_LIBDIR = $(abspath lib/sdl2w)
SDL2W_MODULES_DIR = $(abspath lib/sdl2w/modules)
BMIN_MODULES_DIR = $(abspath lib/sdl2w/modules/bmin)

# Prefer copied consumer bundle; fall back to in-tree sdl2w modules during bootstrap.
ifeq ($(wildcard lib/sdl2w/modules/make/use.mk),)
  ifneq ($(wildcard $(SDL2W_DIR_NAME)/src/modules/make/use.mk),)
    include $(SDL2W_DIR_NAME)/src/modules/make/use.mk
  endif
else
  include lib/sdl2w/modules/make/use.mk
endif

DEBUG = true
LINK_FLAGS = -fuse-ld=lld

ifeq ($(TARGET),wasm)
  EXE_SUFFIX = .js
  CXX = em++
  AR = emar
  FLAGS = -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Oz
else
  ifndef CXX
    CXX = g++
  endif
  ifndef AR
    AR = ar
  endif
  FLAGS = -Og -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -fno-omit-frame-pointer
  ifeq ($(DEBUG),true)
    FLAGS += -g
  endif
endif

# Game flags = project warnings + sdl2w module flags (adds -std=c++23 -fmodules-ts -I…).
CARCER_CXXFLAGS = $(FLAGS) $(SDL2W_CXXFLAGS) -Imodules
CARCER_LDLIBS = $(SDL2W_LDLIBS)

CARCER_BMI_OBJS = $(shell cat modules/bmi_objs.list 2>/dev/null)
CARCER_IFACES = $(shell cat modules/cppm_sources.list 2>/dev/null)

SOURCES = $(CODE) $(MAIN_ALL)

# Do not -include old header-era .d files (modules break those deps).


.PHONY: all sdl2w sdl2w_wasm sdl2w_rebuild carcer-bmi libcarcer object_files \
	print_cxx print_sources print_compile_flags print_compile_db compiler_args \
	compiler_link_args \
	run js debug clean _copy_sdl2w_artifacts

all: sdl2w
	$(MAKE) carcer-bmi
	$(MAKE) $(EXE)

sdl2w: $(SDL2W_MODULES_LIB)

sdl2w_rebuild:
	$(MAKE) -C "$(SDL2W_DIR_NAME)/src" native
	@$(MAKE) _copy_sdl2w_artifacts

$(SDL2W_STAMP):
	@if [ ! -d $(SDL2W_DIR_NAME) ]; then \
		echo "Cloning $(SDL2W_REPO_URL) into $(SDL2W_DIR_NAME)..."; \
		git clone $(SDL2W_REPO_URL) $(SDL2W_DIR_NAME); \
		cd $(SDL2W_DIR_NAME) && git checkout $(SDL2W_REF); \
	else \
		echo "Using existing $(SDL2W_DIR_NAME)"; \
	fi
	@touch $@

$(SDL2W_MODULES_LIB): $(SDL2W_STAMP)
	@echo "Building sdl2w modules (make native in $(SDL2W_DIR_NAME)/src)..."
	$(MAKE) -C "$(SDL2W_DIR_NAME)/src" native
	@$(MAKE) _copy_sdl2w_artifacts

sdl2w_wasm: $(SDL2W_STAMP)
	@echo "Building sdl2w modules for wasm..."
	$(MAKE) -C "$(SDL2W_DIR_NAME)/src" TARGET=wasm
	@$(MAKE) _copy_sdl2w_artifacts

_copy_sdl2w_artifacts:
	@mkdir -p lib/sdl2w
	$(SDL2W_DIR_NAME)/copy-sdl2w-artifacts.sh lib/sdl2w

carcer-bmi: $(CARCER_BMI_STAMP)

$(CARCER_BMI_STAMP): $(SDL2W_BMI_STAMP) $(CARCER_IFACES) modules/make/build-bmi.mk modules/bmi_objs.list
	$(MAKE) -f modules/make/build-bmi.mk CXX=$(CXX) -j1
	@touch $(CARCER_BMI_STAMP)

$(LIBCARCER): $(OBJECTS) $(CARCER_BMI_STAMP)
	@mkdir -p lib
	$(file >lib/libcarcer.objs,$(OBJECTS) $(CARCER_BMI_OBJS))
	$(AR) rcs $@ @lib/libcarcer.objs

# Recipe-based, not a plain prerequisite forward: carcer-bmi must fully
# complete as its own already-exited $(MAKE) process before the $(LIBCARCER)
# invocation even starts checking $(OBJECTS). In one -j process, GNU Make can
# "consider" a .cpp target's .carcer-bmi/*.o prerequisite before a *sibling*
# target's dependency chain has finished rebuilding that same file as a
# recursive-submake side effect, and keep the pre-rebuild mtime for the first
# target's freshness check - a real scheduling/caching gap, not a misreading
# of the docs (reproduced directly with `make --debug=v`). Splitting into two
# sequential process invocations sidesteps it entirely: the second process
# stats every BMI object fresh, and none of them change during its lifetime.
libcarcer object_files:
	$(MAKE) carcer-bmi
	$(MAKE) $(LIBCARCER)

$(EXE): $(MAIN_ALL_OBJECT) $(LIBCARCER) $(CARCER_BMI_STAMP) $(SDL2W_BMI_STAMP)
ifeq ($(TARGET),wasm)
	$(CXX) $(CARCER_CXXFLAGS) $(LINK_FLAGS) -o $(EXE)$(EXE_SUFFIX) \
		$(MAIN_ALL_OBJECT) -Wl,--start-group $(LIBCARCER) -Wl,--end-group $(CARCER_LDLIBS)
else
	$(CXX) $(CARCER_CXXFLAGS) $(LINK_FLAGS) -o $(EXE)$(EXE_SUFFIX) \
		$(MAIN_ALL_OBJECT) -Wl,--start-group $(LIBCARCER) -Wl,--end-group $(CARCER_LDLIBS)
endif

# Per-file BMI dependencies (which .carcer-bmi/*.o each impl unit actually
# imports) come from modules/cpp_bmi_deps.mk, regenerated by
# scripts/modules/gen_bmi_makefile.py alongside build-bmi.mk. `carcer-bmi`
# stays an order-only prerequisite: it guarantees every BMI is built/fresh
# before any .cpp compiles, without itself (a .PHONY target) forcing every
# .cpp to look stale the way depending on it normally would.
-include modules/cpp_bmi_deps.mk

# cpp_bmi_deps.mk lists specific .carcer-bmi/*.o files as normal prerequisites
# (for real per-file staleness checks), but those objects are only ever
# actually produced by the recursive submake in the carcer-bmi/$(CARCER_BMI_STAMP)
# recipe below - this make instance has no rule of its own for them. Give it
# one: order-only on carcer-bmi guarantees that submake runs (and the real
# object gets written) before this placeholder is even evaluated, every time,
# regardless of whether *this* rule's own no-op recipe ends up running.
.carcer-bmi/%.o: | carcer-bmi
	@:

%.o: %.cpp | carcer-bmi
	$(CXX) $(CARCER_CXXFLAGS) -c $< -o $@

# GCC modules may emit fictional *.c++-module make prerequisites; ignore them.
%.c++-module: ;

compiler_args:
	@echo $(CARCER_CXXFLAGS) $(LINK_FLAGS) -Wl,--start-group $(LIBCARCER) -Wl,--end-group $(CARCER_LDLIBS)

compiler_link_args:
	@echo $(LINK_FLAGS) -Wl,--start-group $(LIBCARCER) -Wl,--end-group $(CARCER_LDLIBS)
print_cxx:
	@echo $(CXX)

print_sources:
	@printf '%s\n' $(SOURCES)

print_compile_flags:
	@echo $(CARCER_CXXFLAGS)

print_compile_db:
	@printf 'CXX:%s\n' '$(CXX)'
	@printf 'FLAGS:%s\n' '$(CARCER_CXXFLAGS)'
	@for src in $(SOURCES); do printf 'SOURCE:%s\n' "$$src"; done
	@for m in $(CARCER_IFACES); do \
		printf 'MODULE:%s\n' "$$m"; \
	done

# Module builds: skip classic -include $(DEPENDS); BMI stamps gate rebuilds.

run:
	./$(EXE)$(EXE_SUFFIX)

js: sdl2w_wasm
	$(MAKE) TARGET=wasm $(EXE)
	mv -f $(EXE).js ../web/
	mv -f $(EXE).data ../web/
	mv -f $(EXE).wasm ../web/

debug:
	gdb $(EXE)

clean:
	rm -f $(OBJECTS) $(MAIN_ALL_OBJECT)
	rm -f $(LIBCARCER)
	rm -f lib/libcarcer.objs
	rm -f $(DEPENDS)
	rm -f $(EXE) $(EXE).exe $(EXE).js $(EXE).wasm $(EXE).data
	rm -rf main.d
	rm -rf gcm.cache pcm.cache .bmin-bmi .sdl2w-bmi .carcer-bmi
	# Keep lib/sdl2w module artifacts; use sdl2w_rebuild to refresh them.
	rm -rf $(SDL2W_STAMP)
