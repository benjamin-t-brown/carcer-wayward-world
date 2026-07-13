#pragma once

#include "layers/ui/LayerWorld.h"
#include "state/WorldActions.h"
#include "ui/UiElement.h"

namespace ui {

class ObserverWorldAction : public UiEventObserver {
  layers::LayerWorld* layerWorld;
  state::WorldActionType worldActionType;

public:
  ObserverWorldAction(layers::LayerWorld* _layerWorld,
                      state::WorldActionType _worldActionType)
      : layerWorld(_layerWorld), worldActionType(_worldActionType) {}

  void onClick(int mouseX, int mouseY, int button) override {
    if (layerWorld) {
      layerWorld->activateWorldAction(worldActionType);
    }
  }
};

} // namespace ui
