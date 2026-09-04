module;
#include <cstddef>

export module carcer.actions.ui.layers:UiShowLayerEquipRunes;
export import carcer.state;
import sdl2w;
#include "macros.h"

export {

namespace state {

namespace actions {

class UiShowLayerEquipRunes : public AbstractAction {
  bmin::String characterPlayerId;

  void act() override {
    if (!state) {
      return;
    }
    pushLayerRequest(*state,
                     LayerRequest{.id = LayerId::EquipRunes, .a = characterPlayerId});
  }

public:
  UiShowLayerEquipRunes(sdl2w::Window* /*_window*/, const bmin::String& _characterPlayerId)
      : characterPlayerId(_characterPlayerId) {}
};

} // namespace actions

} // namespace state

} // export
