#pragma once

#include "layers/LayerManager.h"
#include "layers/ui/LayerSpellCast.h"
#include "state/AbstractAction.h"

namespace state {

namespace actions {

class UiShowLayerSpellCast : public AbstractAction {
  sdl2w::Window* window;
  bmin::String chId;

  void act() override {
    auto layerManager = getLayerManager();
    if (!layerManager) {
      return;
    }
    auto existing = layerManager->getLayerById(layers::LayerSpellCast::LAYER_ID);
    if (existing != nullptr) {
      layerManager->moveToFront(existing);
      return;
    }
    auto layer = new layers::LayerSpellCast(window, chId);
    layerManager->addLayer(layer);
    layerManager->moveToFront(layer);
  }

public:
  explicit UiShowLayerSpellCast(sdl2w::Window* _window, const bmin::String& chId)
      : window(_window), chId(chId) {}
};

} // namespace actions

} // namespace state
