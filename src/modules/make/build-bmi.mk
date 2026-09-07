CXX ?= g++
CARCER_MOD ?= modules
SDL2W_MOD ?= lib/sdl2w/modules
BMIN_MOD ?= lib/sdl2w/modules/bmin
CARCER_BMI_FLAGS ?= -Wall -std=c++23 -g -fmodules-ts -I$(CARCER_MOD) -I$(SDL2W_MOD) -I$(BMIN_MOD)
OBJDIR = .carcer-bmi

.PHONY: all clean

all: $(OBJDIR)/carcer.o
	@mkdir -p gcm.cache
	@touch gcm.cache/.carcer-ready

$(OBJDIR):
	@mkdir -p $@

$(OBJDIR)/carcer.data.o: data/data.cppm | $(OBJDIR)
	$(CXX) $(CARCER_BMI_FLAGS) -c data/data.cppm -o $@

$(OBJDIR)/carcer.game.map.TileFields.o: game/map/TileFields.cppm | $(OBJDIR)
	$(CXX) $(CARCER_BMI_FLAGS) -c game/map/TileFields.cppm -o $@

$(OBJDIR)/carcer.lib.Json.o: lib/Json.cppm | $(OBJDIR)
	$(CXX) $(CARCER_BMI_FLAGS) -c lib/Json.cppm -o $@

$(OBJDIR)/carcer.lib.StringUtil.o: lib/StringUtil.cppm | $(OBJDIR)
	$(CXX) $(CARCER_BMI_FLAGS) -c lib/StringUtil.cppm -o $@

$(OBJDIR)/carcer.lib.hiscore.hiscore.o: lib/hiscore/_hiscore.cppm | $(OBJDIR)
	$(CXX) $(CARCER_BMI_FLAGS) -c lib/hiscore/_hiscore.cppm -o $@

$(OBJDIR)/carcer.in3.o: in3/_in3.cppm $(OBJDIR)/carcer.data.o | $(OBJDIR)
	$(CXX) $(CARCER_BMI_FLAGS) -c in3/_in3.cppm -o $@

$(OBJDIR)/carcer.model.o: model/model.cppm $(OBJDIR)/carcer.data.o $(OBJDIR)/carcer.game.map.TileFields.o | $(OBJDIR)
	$(CXX) $(CARCER_BMI_FLAGS) -c model/model.cppm -o $@

$(OBJDIR)/carcer.db.o: db/_db.cppm $(OBJDIR)/carcer.data.o $(OBJDIR)/carcer.lib.Json.o | $(OBJDIR)
	$(CXX) $(CARCER_BMI_FLAGS) -c db/_db.cppm -o $@

$(OBJDIR)/carcer.game.inventory.o: game/inventory/_inventory.cppm $(OBJDIR)/carcer.db.o $(OBJDIR)/carcer.model.o | $(OBJDIR)
	$(CXX) $(CARCER_BMI_FLAGS) -c game/inventory/_inventory.cppm -o $@

$(OBJDIR)/carcer.game.map.o: game/map/_map.cppm $(OBJDIR)/carcer.data.o $(OBJDIR)/carcer.db.o $(OBJDIR)/carcer.game.map.TileFields.o $(OBJDIR)/carcer.model.o | $(OBJDIR)
	$(CXX) $(CARCER_BMI_FLAGS) -c game/map/_map.cppm -o $@

$(OBJDIR)/carcer.state.o: state/_State.cppm $(OBJDIR)/carcer.data.o $(OBJDIR)/carcer.db.o $(OBJDIR)/carcer.model.o | $(OBJDIR)
	$(CXX) $(CARCER_BMI_FLAGS) -c state/_State.cppm -o $@

$(OBJDIR)/carcer.game.combat.o: game/combat/_combat.cppm $(OBJDIR)/carcer.data.o $(OBJDIR)/carcer.db.o $(OBJDIR)/carcer.game.map.o $(OBJDIR)/carcer.model.o | $(OBJDIR)
	$(CXX) $(CARCER_BMI_FLAGS) -c game/combat/_combat.cppm -o $@

$(OBJDIR)/carcer.actions.o: actions/_actions.cppm $(OBJDIR)/carcer.state.o | $(OBJDIR)
	$(CXX) $(CARCER_BMI_FLAGS) -c actions/_actions.cppm -o $@

$(OBJDIR)/carcer.ui.core.o: ui/_core.cppm $(OBJDIR)/carcer.state.o | $(OBJDIR)
	$(CXX) $(CARCER_BMI_FLAGS) -c ui/_core.cppm -o $@

$(OBJDIR)/carcer.ui.layers.o: ui/_layers.cppm $(OBJDIR)/carcer.state.o $(OBJDIR)/carcer.ui.core.o | $(OBJDIR)
	$(CXX) $(CARCER_BMI_FLAGS) -c ui/_layers.cppm -o $@

$(OBJDIR)/carcer.ui.widgets.foundation.o: ui/_widget_foundation.cppm $(OBJDIR)/carcer.state.o $(OBJDIR)/carcer.ui.core.o | $(OBJDIR)
	$(CXX) $(CARCER_BMI_FLAGS) -c ui/_widget_foundation.cppm -o $@

$(OBJDIR)/carcer.ui.widgets.composites.o: ui/_widget_composites.cppm $(OBJDIR)/carcer.actions.o $(OBJDIR)/carcer.data.o $(OBJDIR)/carcer.game.map.o $(OBJDIR)/carcer.game.map.TileFields.o $(OBJDIR)/carcer.model.o $(OBJDIR)/carcer.state.o $(OBJDIR)/carcer.ui.core.o $(OBJDIR)/carcer.ui.widgets.foundation.o | $(OBJDIR)
	$(CXX) $(CARCER_BMI_FLAGS) -c ui/_widget_composites.cppm -o $@

$(OBJDIR)/carcer.ui.widgets.views.o: ui/_widget_views.cppm $(OBJDIR)/carcer.actions.o $(OBJDIR)/carcer.data.o $(OBJDIR)/carcer.game.map.o $(OBJDIR)/carcer.game.map.TileFields.o $(OBJDIR)/carcer.model.o $(OBJDIR)/carcer.state.o $(OBJDIR)/carcer.ui.core.o $(OBJDIR)/carcer.ui.widgets.foundation.o | $(OBJDIR)
	$(CXX) $(CARCER_BMI_FLAGS) -c ui/_widget_views.cppm -o $@

$(OBJDIR)/carcer.ui.widgets.o: ui/_widgets.cppm $(OBJDIR)/carcer.ui.widgets.composites.o $(OBJDIR)/carcer.ui.widgets.foundation.o $(OBJDIR)/carcer.ui.widgets.views.o | $(OBJDIR)
	$(CXX) $(CARCER_BMI_FLAGS) -c ui/_widgets.cppm -o $@

$(OBJDIR)/carcer.ui.screens.runtime.o: ui/screens/runtime.cppm $(OBJDIR)/carcer.actions.o $(OBJDIR)/carcer.lib.StringUtil.o $(OBJDIR)/carcer.model.o $(OBJDIR)/carcer.state.o $(OBJDIR)/carcer.ui.core.o $(OBJDIR)/carcer.ui.widgets.o | $(OBJDIR)
	$(CXX) $(CARCER_BMI_FLAGS) -c ui/screens/runtime.cppm -o $@

$(OBJDIR)/carcer.ui.screens.layouts.o: ui/screens/layouts.cppm $(OBJDIR)/carcer.actions.o $(OBJDIR)/carcer.state.o $(OBJDIR)/carcer.ui.core.o $(OBJDIR)/carcer.ui.screens.runtime.o $(OBJDIR)/carcer.ui.widgets.o | $(OBJDIR)
	$(CXX) $(CARCER_BMI_FLAGS) -c ui/screens/layouts.cppm -o $@

$(OBJDIR)/carcer.ui.screens.overlays.o: ui/screens/overlays.cppm $(OBJDIR)/carcer.actions.o $(OBJDIR)/carcer.data.o $(OBJDIR)/carcer.model.o $(OBJDIR)/carcer.state.o $(OBJDIR)/carcer.ui.core.o $(OBJDIR)/carcer.ui.screens.layouts.o $(OBJDIR)/carcer.ui.screens.runtime.o $(OBJDIR)/carcer.ui.widgets.o | $(OBJDIR)
	$(CXX) $(CARCER_BMI_FLAGS) -c ui/screens/overlays.cppm -o $@

$(OBJDIR)/carcer.ui.screens.pages.o: ui/screens/pages.cppm $(OBJDIR)/carcer.actions.o $(OBJDIR)/carcer.data.o $(OBJDIR)/carcer.model.o $(OBJDIR)/carcer.state.o $(OBJDIR)/carcer.ui.core.o $(OBJDIR)/carcer.ui.screens.layouts.o $(OBJDIR)/carcer.ui.screens.runtime.o $(OBJDIR)/carcer.ui.widgets.o | $(OBJDIR)
	$(CXX) $(CARCER_BMI_FLAGS) -c ui/screens/pages.cppm -o $@

$(OBJDIR)/carcer.ui.screens.o: ui/_screens.cppm $(OBJDIR)/carcer.ui.screens.layouts.o $(OBJDIR)/carcer.ui.screens.overlays.o $(OBJDIR)/carcer.ui.screens.pages.o $(OBJDIR)/carcer.ui.screens.runtime.o | $(OBJDIR)
	$(CXX) $(CARCER_BMI_FLAGS) -c ui/_screens.cppm -o $@

$(OBJDIR)/carcer.o: modules/_carcer.cppm $(OBJDIR)/carcer.data.o $(OBJDIR)/carcer.game.map.TileFields.o $(OBJDIR)/carcer.lib.Json.o $(OBJDIR)/carcer.lib.StringUtil.o $(OBJDIR)/carcer.lib.hiscore.hiscore.o $(OBJDIR)/carcer.in3.o $(OBJDIR)/carcer.model.o $(OBJDIR)/carcer.db.o $(OBJDIR)/carcer.game.inventory.o $(OBJDIR)/carcer.game.map.o $(OBJDIR)/carcer.state.o $(OBJDIR)/carcer.game.combat.o $(OBJDIR)/carcer.actions.o $(OBJDIR)/carcer.ui.core.o $(OBJDIR)/carcer.ui.layers.o $(OBJDIR)/carcer.ui.widgets.foundation.o $(OBJDIR)/carcer.ui.widgets.composites.o $(OBJDIR)/carcer.ui.widgets.views.o $(OBJDIR)/carcer.ui.widgets.o $(OBJDIR)/carcer.ui.screens.runtime.o $(OBJDIR)/carcer.ui.screens.layouts.o $(OBJDIR)/carcer.ui.screens.overlays.o $(OBJDIR)/carcer.ui.screens.pages.o $(OBJDIR)/carcer.ui.screens.o | $(OBJDIR)
	$(CXX) $(CARCER_BMI_FLAGS) -c modules/_carcer.cppm -o $@

clean:
	rm -rf $(OBJDIR)

CARCER_BMI_OBJ_LIST = $(OBJDIR)/carcer.data.o $(OBJDIR)/carcer.game.map.TileFields.o $(OBJDIR)/carcer.lib.Json.o $(OBJDIR)/carcer.lib.StringUtil.o $(OBJDIR)/carcer.lib.hiscore.hiscore.o $(OBJDIR)/carcer.in3.o $(OBJDIR)/carcer.model.o $(OBJDIR)/carcer.db.o $(OBJDIR)/carcer.game.inventory.o $(OBJDIR)/carcer.game.map.o $(OBJDIR)/carcer.state.o $(OBJDIR)/carcer.game.combat.o $(OBJDIR)/carcer.actions.o $(OBJDIR)/carcer.ui.core.o $(OBJDIR)/carcer.ui.layers.o $(OBJDIR)/carcer.ui.widgets.foundation.o $(OBJDIR)/carcer.ui.widgets.composites.o $(OBJDIR)/carcer.ui.widgets.views.o $(OBJDIR)/carcer.ui.widgets.o $(OBJDIR)/carcer.ui.screens.runtime.o $(OBJDIR)/carcer.ui.screens.layouts.o $(OBJDIR)/carcer.ui.screens.overlays.o $(OBJDIR)/carcer.ui.screens.pages.o $(OBJDIR)/carcer.ui.screens.o $(OBJDIR)/carcer.o

