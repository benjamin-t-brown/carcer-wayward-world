#pragma once

namespace sdl2w { class Window; }

#include "bmin/String.h"
#include "state/AbstractAction.hpp"
#include "state/State.hpp"

namespace state {

namespace actions {

class UiShowLayerSpecialEvent : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::UiShowLayerSpecialEvent; }
  bmin::String eventId;

  void act() override {
    if (!state) {
      return;
    }
    pushLayerRequest(*state,
                     LayerRequest{.id = LayerId::SpecialEvent, .a = eventId});
  }

public:
  UiShowLayerSpecialEvent(sdl2w::Window* /*window*/, bmin::String _eventId)
      : eventId(std::move(_eventId)) {}
};

} // namespace actions

} // namespace state
