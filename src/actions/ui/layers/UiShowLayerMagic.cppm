module;
#include <cstddef>

export module carcer.actions.ui.layers:UiShowLayerMagic;
export import carcer.state;
import sdl2w;
import carcer.model;
#include "macros.h"

export {

namespace state {

namespace actions {

class UiShowLayerMagic : public AbstractAction {
  void act() override {
    if (!state) {
      return;
    }
    auto& player = state->player;
    const int selectedIndex = model::playerFindPartyMemberIndexById(
        player, state->uiState.selectedPartyMemberId);
    player.currentPartyMemberMagicIndex = selectedIndex >= 0 ? selectedIndex : 0;
    pushLayerRequest(*state, LayerRequest{.id = LayerId::Magic});
  }

public:
  explicit UiShowLayerMagic(sdl2w::Window* /*_window*/) {}
};

} // namespace actions

} // namespace state

} // export
