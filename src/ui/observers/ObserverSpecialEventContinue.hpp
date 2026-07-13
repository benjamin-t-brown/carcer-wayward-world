#pragma once

#include "layers/ui/LayerSpecialEvent.h"
#include "ui/UiElement.h"

namespace ui {

class ObserverSpecialEventContinue : public UiEventObserver {
  layers::LayerSpecialEvent* layer;

public:
  explicit ObserverSpecialEventContinue(layers::LayerSpecialEvent* _layer) : layer(_layer) {}

  void onClick(int mouseX, int mouseY, int button) override {
    if (layer) {
      layer->onContinue();
    }
  }
};

} // namespace ui
