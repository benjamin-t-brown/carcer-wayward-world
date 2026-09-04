module;
#include <cstddef>

export module carcer.actions.ui.layers:UiCancelEquipRunes;
export import carcer.state;
#include "macros.h"

export {

namespace state {

namespace actions {

/** Close the Equip Runes layer without committing edits.
    TODO(reconciler): snapshot restore belongs to the Layer itself (it can
    revert on close) once LayerManager::update() reconciles layerStack --
    see LayerRequest in carcer.state. */
class UiCancelEquipRunes : public AbstractAction {
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
