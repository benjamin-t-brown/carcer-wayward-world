module;
#include <cstddef>

export module carcer.actions.ui.layers:UiCommitEquipRunes;
export import carcer.state;
#include "macros.h"

export {

namespace state {

namespace actions {

/** Keep live equipped-rune edits and close the Equip Runes layer. */
class UiCommitEquipRunes : public AbstractAction {
  void act() override {
    if (!state) {
      return;
    }
    removeLayerRequest(*state, LayerId::EquipRunes);
  }
};

} // namespace actions

} // namespace state

} // export
