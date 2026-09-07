#pragma once

#include "state/StateManager.h"
#include "actions/navigation/UiCommitEquipRunes.hpp"
#include "ui/UiElement.h"

namespace ui {

class ObserverCommitEquipRunes : public ui::UiEventObserver,
                                 public state::StateManagerInterface {
public:
  void onClick(int /*mouseX*/, int /*mouseY*/, int /*button*/) override {
    auto* stateManager = getStateManager();
    if (!stateManager) {
      return;
    }
    stateManager->enqueueAction(
        stateManager->getActionData(), new state::actions::UiCommitEquipRunes(), 0);
  }
};

} // namespace ui
