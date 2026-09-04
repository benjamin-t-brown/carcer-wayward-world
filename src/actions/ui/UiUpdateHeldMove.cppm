module;
#include <utility>
#include <cstddef>

export module carcer.actions.ui:UiUpdateHeldMove;
export import carcer.state;
#include "macros.h"

export {

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

} // export
