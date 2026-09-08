#pragma once

#include "state/AbstractAction.hpp"
#include "state/State.hpp"

namespace state {

namespace actions {

/** Restore equipped runes from the open-editor snapshot and close the layer. */
class UiCancelEquipRunes : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::UiCancelEquipRunes; }
  void act() override {
    if (!state) {
      return;
    }
    removeLayerRequest(*state, LayerId::EquipRunes);
  }
};

} // namespace actions

} // namespace state
