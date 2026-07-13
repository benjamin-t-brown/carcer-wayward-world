#pragma once

#include "layers/ui/LayerWorld.h"
#include "ui/UiElement.h"

namespace ui {

class ObserverCancelWorldActionMode : public UiEventObserver {
  layers::LayerWorld* layerWorld;

public:
  explicit ObserverCancelWorldActionMode(layers::LayerWorld* _layerWorld)
      : layerWorld(_layerWorld) {}

  void onClick(int /*mouseX*/, int /*mouseY*/, int /*button*/) override {
    if (layerWorld) {
      layerWorld->cancelWorldActionMode();
    }
  }
};

} // namespace ui
