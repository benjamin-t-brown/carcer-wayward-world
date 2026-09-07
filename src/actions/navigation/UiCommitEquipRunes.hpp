#pragma once

#include "state/AbstractAction.h"
#include "state/State.h"

namespace state {

namespace actions {

/** Keep live equipped-rune edits and close the Equip Runes layer. */
class UiCommitEquipRunes : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::UiCommitEquipRunes; }
  void act() override {
    if (!state) {
      return;
    }
    removeLayerRequest(*state, LayerId::EquipRunes);
  }
};

} // namespace actions

} // namespace state
