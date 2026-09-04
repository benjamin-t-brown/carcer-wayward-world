module;
#include <cstddef>

export module carcer.actions.ui.layers:UiShowLayerSpellCast;
export import carcer.state;
import sdl2w;
#include "macros.h"

export {

namespace state {

namespace actions {

class UiShowLayerSpellCast : public AbstractAction {
  bmin::String chId;

  void act() override {
    if (!state) {
      return;
    }
    pushLayerRequest(*state, LayerRequest{.id = LayerId::SpellCast, .a = chId});
  }

public:
  explicit UiShowLayerSpellCast(sdl2w::Window* /*_window*/, const bmin::String& chId)
      : chId(chId) {}
};

} // namespace actions

} // namespace state

} // export
