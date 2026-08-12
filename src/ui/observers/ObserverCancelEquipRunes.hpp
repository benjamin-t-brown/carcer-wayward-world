#pragma once

#include "state/StateManager.h"
#include "state/actions/ui/UiCancelEquipRunes.hpp"
#include "ui/UiElement.h"

namespace ui {

class ObserverCancelEquipRunes : public ui::UiEventObserver,
                                 public state::StateManagerInterface {
public:
  void onClick(int /*mouseX*/, int /*mouseY*/, int /*button*/) override {
    auto* stateManager = getStateManager();
    if (!stateManager) {
      return;
    }
    stateManager->enqueueAction(
        stateManager->getActionData(), new state::actions::UiCancelEquipRunes(), 0);
  }
};

} // namespace ui
