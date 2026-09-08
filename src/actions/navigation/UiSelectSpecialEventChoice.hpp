#pragma once

#include "state/AbstractAction.hpp"

namespace state::actions {

// Broadcast-only. The active special-event layer owns runner state.
class UiSelectSpecialEventChoice : public AbstractAction {
  ActionEvent getEvent() const override {
    return ActionEvent::UiSelectSpecialEventChoice;
  }
  int getEventValue() const override { return choiceIndex; }

public:
  int choiceIndex = -1;

  explicit UiSelectSpecialEventChoice(int choiceIndex)
      : choiceIndex(choiceIndex) {}
};

} // namespace state::actions
