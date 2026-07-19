#pragma once

#include "state/StateManager.h"
#include "ui/UiElement.h"
#include "ui/helpers/worldActions.h"

namespace ui {

class ObserverCancelWorldActionMode : public UiEventObserver {
  state::StateManager* stateManager;

public:
  explicit ObserverCancelWorldActionMode(state::StateManager* _stateManager)
      : stateManager(_stateManager) {}

  void onClick(int /*mouseX*/, int /*mouseY*/, int /*button*/) override {
    if (stateManager) {
      cancelCurrentWorldActionMode(*stateManager);
    }
  }
};

} // namespace ui
