#pragma once

#include "bmin/String.h"
#include "layers/LayerManager.h"
#include "layers/ui/LayerPopupText.h"
#include "state/AbstractAction.h"

namespace state {

namespace actions {

class UiShowLayerPopupText : public AbstractAction {
  sdl2w::Window* window;
  bmin::String title;
  bmin::String text;

  void act() override {
    auto layerManager = getLayerManager();
    if (!layerManager) {
      return;
    }
    if (auto* existing = layerManager->getLayerById(layers::LayerPopupText::LAYER_ID)) {
      existing->remove();
    }
    auto* layer = new layers::LayerPopupText(window, title, text);
    layerManager->addLayer(layer);
    layerManager->moveToFront(layer);
  }

public:
  UiShowLayerPopupText(sdl2w::Window* _window, bmin::String _title, bmin::String _text)
      : window(_window), title(std::move(_title)), text(std::move(_text)) {}
};

} // namespace actions

} // namespace state
