module;
#include <cstddef>

export module carcer.actions.ui.layers:UiShowLayerSpellInfo;
export import carcer.state;
import sdl2w;
#include "macros.h"

export {

namespace state {

namespace actions {

class UiShowLayerSpellInfo : public AbstractAction {
  bmin::String spellName;

  void act() override {
    if (!state) {
      return;
    }
    pushLayerRequest(*state, LayerRequest{.id = LayerId::SpellInfo, .a = spellName});
  }

public:
  UiShowLayerSpellInfo(sdl2w::Window* /*_window*/, const bmin::String& _spellName)
      : spellName(_spellName) {}
};

} // namespace actions

} // namespace state

} // export
