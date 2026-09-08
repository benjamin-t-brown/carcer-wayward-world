#pragma once

#include "state/AbstractAction.hpp"
#include "state/State.hpp"

namespace state {

namespace actions {

class UiUpdateHeldMove : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::UiUpdateHeldMove; }
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
