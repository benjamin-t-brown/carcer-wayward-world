#pragma once

#include "state/AbstractAction.h"
#include "state/State.h"

namespace state {

namespace actions {

class UiUpdateHeldMove : public AbstractAction {
  HeldMove nextHeldMove;

  void act() override {
    auto& localState = *state;
    localState.uiState.heldMove = nextHeldMove;
  }

public:
  explicit UiUpdateHeldMove(HeldMove _nextHeldMove)
      : nextHeldMove(std::move(_nextHeldMove)) {}
};

} // namespace actions

} // namespace state
