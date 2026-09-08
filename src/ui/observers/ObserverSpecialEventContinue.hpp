#pragma once

#include "actions/navigation/UiContinueSpecialEvent.hpp"
#include "state/StateManager.h"
#include "ui/UiElement.h"

namespace ui {

class ObserverSpecialEventContinue : public UiEventObserver,
                                     public state::StateManagerInterface {
public:
  void onClick(int /*mouseX*/, int /*mouseY*/, int /*button*/) override {
    if (auto* stateManager = getStateManager()) {
      stateManager->enqueueAction(state::makeAction<state::actions::UiContinueSpecialEvent>(),
          0);
    }
  }
};

} // namespace ui
