#pragma once

namespace sdl2w { class Window; }

#include "bmin/String.h"
#include "state/AbstractAction.hpp"
#include "state/State.hpp"

namespace state {

namespace actions {

class UiShowLayerPopupText : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::UiShowLayerPopupText; }
  bmin::String title;
  bmin::String text;

  void act() override {
    if (!state) {
      return;
    }
    pushLayerRequest(
        *state, LayerRequest{.id = LayerId::PopupText, .a = title, .b = text});
  }

public:
  UiShowLayerPopupText(sdl2w::Window* /*window*/,
                       bmin::String _title,
                       bmin::String _text)
      : title(std::move(_title)), text(std::move(_text)) {}
};

} // namespace actions

} // namespace state
