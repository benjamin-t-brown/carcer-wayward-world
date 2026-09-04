module;
#include <cstddef>

export module carcer.actions.ui.layers:UiShowLayerInventory;
export import carcer.state;
import sdl2w;
import carcer.model.instances;
#include "macros.h"

export {

namespace state {

namespace actions {

class UiShowLayerInventory : public AbstractAction {
  void act() override {
    if (!state) {
      return;
    }
    auto& player = state->player;
    const int selectedIndex = model::playerFindPartyMemberIndexById(
        player, state->uiState.selectedPartyMemberId);
    player.currentPartyMemberInventoryIndex = selectedIndex >= 0 ? selectedIndex : 0;
    pushLayerRequest(*state, LayerRequest{.id = LayerId::Inventory});
  }

public:
  explicit UiShowLayerInventory(sdl2w::Window* /*_window*/) {}
};

} // namespace actions

} // namespace state

} // export
