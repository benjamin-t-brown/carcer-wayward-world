#pragma once

#include "layers/ui/LayerSpecialEvent.h"
#include "ui/UiElement.h"

namespace ui {

class ObserverSpecialEventChoice : public UiEventObserver {
  layers::LayerSpecialEvent* layer;
  int choiceIndex;

public:
  ObserverSpecialEventChoice(layers::LayerSpecialEvent* _layer, int _choiceIndex)
      : layer(_layer), choiceIndex(_choiceIndex) {}

  void onClick(int mouseX, int mouseY, int button) override {
    if (layer) {
      layer->onChoiceSelected(choiceIndex);
    }
  }
};

} // namespace ui
