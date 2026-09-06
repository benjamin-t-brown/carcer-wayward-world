EXE=CARCER
EXE_SUFFIX=

MAIN_ALL=main.cpp

# Implementation units (.cpp) sit next to their interface units (.cppm).
# Umbrella + macros.h live in modules/. BMI graph: modules/make/build-bmi.mk.
CODE=\
data/templates.cpp \
data/stats.cpp \
model/characters.cpp \
model/maps.cpp \
model/world.cpp \
model/combat.cpp \
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

PROJECT_ROOT := $(abspath ..)
DEPS_ROOT ?= $(PROJECT_ROOT)/.deps
DEPS_LOCK ?= $(PROJECT_ROOT)/deps.lock
DEPS_BOOTSTRAP := $(PROJECT_ROOT)/scripts/bootstrap-deps.sh
DEPS_BUILDER := $(PROJECT_ROOT)/scripts/build-deps.sh
SDL2W_DIR_NAME := $(DEPS_ROOT)/sdl2w
BMIN_DIR_NAME := $(DEPS_ROOT)/bmin
SDL2W_BUNDLE_DIR = lib/sdl2w
SDL2W_MODULES_LIB = $(SDL2W_BUNDLE_DIR)/lib/libsdl2w_modules.a

MAIN_ALL_OBJECT=$(MAIN_ALL:.cpp=.o)
OBJECTS=$(CODE:.cpp=.o)
DEPENDS := $(patsubst %.cpp,%.d,$(CODE)) main.d
LIBCARCER = lib/libcarcer.a
CARCER_BMI_STAMP = gcm.cache/.carcer-ready

# Prefer copied consumer bundle; fall back to in-tree sdl2w modules during bootstrap.
ifeq ($(wildcard $(SDL2W_BUNDLE_DIR)/modules/make/use.mk),)
  ifneq ($(wildcard $(SDL2W_DIR_NAME)/src/modules/make/use.mk),)
    BMIN_REPO_DIR := $(BMIN_DIR_NAME)
    include $(SDL2W_DIR_NAME)/src/modules/make/use.mk
  endif
else
  # copy-sdl2w-artifacts.sh installs archives under DEST/lib and modules under
  # DEST/modules. Override use.mk's defaults explicitly for that bundle.
  SDL2W_LIB = $(abspath $(SDL2W_BUNDLE_DIR)/lib/libsdl2w_modules.a)
  BMIN_LIB = $(abspath $(SDL2W_BUNDLE_DIR)/lib/libbmin_modules.a)
  SDL2W_LIBDIR = $(abspath $(SDL2W_BUNDLE_DIR)/lib)
  BMIN_LIBDIR = $(abspath $(SDL2W_BUNDLE_DIR)/lib)
  SDL2W_MODULES_DIR = $(abspath $(SDL2W_BUNDLE_DIR)/modules)
  BMIN_MODULES_DIR = $(abspath $(SDL2W_BUNDLE_DIR)/modules/bmin)
  include $(SDL2W_BUNDLE_DIR)/modules/make/use.mk
endif

DEBUG = true

# GNU archive groups and generic LLD are ELF conventions. Homebrew GCC on
# macOS supplies the SDK/platform metadata when it drives the system linker,
# and ld64 resolves the project archive without group markers.
ifeq ($(TARGET),wasm)
  LINK_FLAGS = -fuse-ld=lld
  CARCER_ARCHIVE_LINK = -Wl,--start-group $(LIBCARCER) -Wl,--end-group
else ifeq ($(shell uname -s),Darwin)
  LINK_FLAGS =
  CARCER_ARCHIVE_LINK = $(LIBCARCER)
else
  LINK_FLAGS = -fuse-ld=lld
  CARCER_ARCHIVE_LINK = -Wl,--start-group $(LIBCARCER) -Wl,--end-group
endif

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


.PHONY: all deps-bootstrap deps-validate sdl2w sdl2w_headers sdl2w_wasm \
	sdl2w_rebuild carcer-bmi _carcer-bmi libcarcer object_files \
	print_cxx print_sources print_compile_flags print_compile_db compiler_args \
	compiler_link_args run js debug clean

all: sdl2w
	$(MAKE) carcer-bmi
	$(MAKE) $(EXE)

deps-bootstrap:
	CARCER_DEPS_LOCK="$(DEPS_LOCK)" CARCER_DEPS_ROOT="$(DEPS_ROOT)" \
		"$(DEPS_BOOTSTRAP)"

deps-validate:
	CARCER_DEPS_LOCK="$(DEPS_LOCK)" \
		CARCER_DEPS_ROOT="$(DEPS_ROOT)" \
		CARCER_ALLOW_DIRTY_DEPS="$(CARCER_ALLOW_DIRTY_DEPS)" \
		"$(DEPS_BOOTSTRAP)" --check

sdl2w: deps-validate
	CARCER_DEPS_LOCK="$(DEPS_LOCK)" \
		CARCER_DEPS_ROOT="$(DEPS_ROOT)" \
		CARCER_SDL2W_BUNDLE="$(abspath $(SDL2W_BUNDLE_DIR))" \
		CARCER_DEPS_CXX="$(CXX)" \
		CARCER_DEPS_TARGET="$(TARGET)" \
		CARCER_DEPS_MODULE_CXXFLAGS="$(SDL2W_MODULE_CXXFLAGS)" \
		CARCER_DEPS_MODULE_INTERFACE_FLAGS="$(SDL2W_MODULE_INTERFACE_FLAGS)" \
		CARCER_ALLOW_DIRTY_DEPS="$(CARCER_ALLOW_DIRTY_DEPS)" \
		"$(DEPS_BUILDER)" dual

sdl2w_headers: deps-validate
	CARCER_DEPS_LOCK="$(DEPS_LOCK)" \
		CARCER_DEPS_ROOT="$(DEPS_ROOT)" \
		CARCER_SDL2W_BUNDLE="$(abspath $(SDL2W_BUNDLE_DIR))" \
		CARCER_DEPS_CXX="$(CXX)" \
		CARCER_DEPS_TARGET="$(TARGET)" \
		CARCER_ALLOW_DIRTY_DEPS="$(CARCER_ALLOW_DIRTY_DEPS)" \
		"$(DEPS_BUILDER)" headers

sdl2w_rebuild: deps-validate
	CARCER_DEPS_LOCK="$(DEPS_LOCK)" \
		CARCER_DEPS_ROOT="$(DEPS_ROOT)" \
		CARCER_SDL2W_BUNDLE="$(abspath $(SDL2W_BUNDLE_DIR))" \
		CARCER_DEPS_CXX="$(CXX)" \
		CARCER_DEPS_TARGET="$(TARGET)" \
		CARCER_DEPS_MODULE_CXXFLAGS="$(SDL2W_MODULE_CXXFLAGS)" \
		CARCER_DEPS_MODULE_INTERFACE_FLAGS="$(SDL2W_MODULE_INTERFACE_FLAGS)" \
		CARCER_ALLOW_DIRTY_DEPS="$(CARCER_ALLOW_DIRTY_DEPS)" \
		CARCER_DEPS_FORCE=1 \
		"$(DEPS_BUILDER)" dual

sdl2w_wasm:
	$(MAKE) TARGET=wasm sdl2w

carcer-bmi: sdl2w
	$(MAKE) _carcer-bmi

_carcer-bmi: $(CARCER_BMI_STAMP)

$(CARCER_BMI_STAMP): $(SDL2W_BMI_STAMP) $(CARCER_IFACES) modules/make/build-bmi.mk modules/bmi_objs.list
	$(MAKE) -f modules/make/build-bmi.mk CXX=$(CXX) \
		CARCER_BMI_FLAGS="$(CARCER_CXXFLAGS)" -j1
	@touch $(CARCER_BMI_STAMP)

$(LIBCARCER): Makefile modules/bmi_objs.list $(OBJECTS) $(CARCER_BMI_STAMP)
	@mkdir -p lib
	$(AR) rcs $@ $(OBJECTS) $(CARCER_BMI_OBJS)

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
libcarcer object_files: sdl2w
	$(MAKE) carcer-bmi
	$(MAKE) $(LIBCARCER)

$(EXE): $(MAIN_ALL_OBJECT) $(LIBCARCER) $(CARCER_BMI_STAMP) $(SDL2W_BMI_STAMP)
	$(CXX) $(CARCER_CXXFLAGS) $(LINK_FLAGS) -o $(EXE)$(EXE_SUFFIX) \
		$(MAIN_ALL_OBJECT) $(CARCER_ARCHIVE_LINK) $(CARCER_LDLIBS)

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
	@echo $(CARCER_CXXFLAGS) $(LINK_FLAGS) $(CARCER_ARCHIVE_LINK) $(CARCER_LDLIBS)

compiler_link_args:
	@echo $(LINK_FLAGS) $(CARCER_ARCHIVE_LINK) $(CARCER_LDLIBS)
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
	# Keep the pinned lib/sdl2w bundle; use sdl2w_rebuild to refresh it.
