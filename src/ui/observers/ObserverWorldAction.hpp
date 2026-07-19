#pragma once

#include "state/StateManager.h"
#include "state/WorldActions.h"
#include "ui/UiElement.h"
#include "ui/helpers/worldActions.h"

namespace ui {

class ObserverWorldAction : public UiEventObserver {
  state::StateManager* stateManager;
  state::WorldActionType worldActionType;

public:
  ObserverWorldAction(state::StateManager* _stateManager,
                      state::WorldActionType _worldActionType)
      : stateManager(_stateManager), worldActionType(_worldActionType) {}

  void onClick(int mouseX, int mouseY, int button) override {
    if (stateManager) {
      activateWorldAction(*stateManager, worldActionType);
    }
  }
};

} // namespace ui
