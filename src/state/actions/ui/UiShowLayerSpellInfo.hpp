#pragma once

#include "bmin/String.h"
#include "layers/LayerManager.h"
#include "layers/ui/LayerSpellInfo.h"
#include "state/AbstractAction.h"

namespace state {

namespace actions {

class UiShowLayerSpellInfo : public AbstractAction {
  sdl2w::Window* window;
  bmin::String spellName;

  void act() override {
    auto layerManager = getLayerManager();
    if (!layerManager) {
      return;
    }
    if (auto* existing = layerManager->getLayerById(layers::LayerSpellInfo::LAYER_ID)) {
      existing->remove();
    }
    auto* layer = new layers::LayerSpellInfo(window, spellName);
    layerManager->addLayer(layer);
    layerManager->moveToFront(layer);
  }

public:
  UiShowLayerSpellInfo(sdl2w::Window* _window, const bmin::String& _spellName)
      : window(_window), spellName(_spellName) {}
};

} // namespace actions

} // namespace state
