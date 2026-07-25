#pragma once

#include "state/StateManager.h"
#include "state/WorldActions.h"
#include "ui/UiElement.h"
#include "ui/helpers/worldActions.h"

namespace ui {

class ObserverWorldAction : public UiEventObserver {
  state::StateManager* stateManager;
  state::WorldActionType worldActionType;
  sdl2w::Window* window;

public:
  ObserverWorldAction(state::StateManager* _stateManager,
                      state::WorldActionType _worldActionType,
                      sdl2w::Window* _window)
      : stateManager(_stateManager),
        worldActionType(_worldActionType),
        window(_window) {}

  void onClick(int mouseX, int mouseY, int button) override {
    if (stateManager) {
      activateWorldAction(*stateManager, worldActionType, window);
    }
  }
};

} // namespace ui
