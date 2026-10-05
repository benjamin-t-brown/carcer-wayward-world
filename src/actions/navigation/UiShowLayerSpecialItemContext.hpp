#pragma once

namespace sdl2w { class Window; }

#include "bmin/String.h"
#include "state/AbstractAction.hpp"
#include "state/State.hpp"

namespace state {

namespace actions {

class UiShowLayerSpecialItemContext : public AbstractAction {
  ActionEvent getEvent() const override {
    return ActionEvent::UiShowLayerSpecialItemContext;
  }
  bmin::String itemTemplateName;
  int quantity = 1;
  void act() override {
    if (!state) {
      return;
    }
    pushLayerRequest(
        *state,
        LayerRequest{.id = LayerId::SpecialItemContext,
                     .a = itemTemplateName,
                     .b = bmin::toString(quantity)});
  }

public:
  UiShowLayerSpecialItemContext(sdl2w::Window* /*window*/,
                                bmin::String itemTemplateName,
                                int quantity)
      : itemTemplateName(itemTemplateName), quantity(quantity) {}
};

} // namespace actions

} // namespace state
