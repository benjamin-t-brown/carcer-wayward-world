#pragma once

#include "bmin/String.h"
#include "bmin/StringInterop.h"
#include "layers/LayerManager.h"
#include "layers/ui/LayerSpecialEvent.h"
#include "state/AbstractAction.h"
#include "state/DatabaseInterface.h"

namespace state {

namespace actions {

class UiShowLayerSpecialEvent : public AbstractAction {
  sdl2w::Window* window;
  bmin::String eventId;

  void act() override {
    auto* database = getDatabase();
    auto layerManager = getLayerManager();
    if (!database || !layerManager) {
      return;
    }

    if (auto* existing =
            layerManager->getLayerById(layers::LayerSpecialEvent::LAYER_ID)) {
      existing->remove();
    }

    const auto& gameEvent = database->getGameEvent(eventId.sliceView());
    auto* layer = new layers::LayerSpecialEvent(
        window, gameEvent, database->getGameEvents(), state->world.specialEventStorage);
    layerManager->addLayer(layer);
    layerManager->moveToFront(layer);
  }

public:
  UiShowLayerSpecialEvent(sdl2w::Window* _window, bmin::String _eventId)
      : window(_window), eventId(std::move(_eventId)) {}
};

} // namespace actions

} // namespace state
