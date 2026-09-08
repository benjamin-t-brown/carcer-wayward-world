#pragma once

#include "actions/navigation/UiSelectSpecialEventChoice.hpp"
#include "state/StateManager.h"
#include "ui/UiElement.h"

namespace ui {

class ObserverSpecialEventChoice : public UiEventObserver,
                                   public state::StateManagerInterface {
  int choiceIndex;

public:
  explicit ObserverSpecialEventChoice(int choiceIndex)
      : choiceIndex(choiceIndex) {}

  void onClick(int /*mouseX*/, int /*mouseY*/, int /*button*/) override {
    if (auto* stateManager = getStateManager()) {
      stateManager->enqueueAction(state::makeAction<state::actions::UiSelectSpecialEventChoice>(choiceIndex),
          0);
    }
  }
};

} // namespace ui
