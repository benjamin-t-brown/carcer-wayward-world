#pragma once

#include "state/AbstractAction.h"

namespace state::actions {

// Broadcast-only. The active special-event layer owns runner state.
class UiContinueSpecialEvent : public AbstractAction {
  ActionEvent getEvent() const override {
    return ActionEvent::UiContinueSpecialEvent;
  }
};

} // namespace state::actions
