module;
#include <utility>
#include <cstddef>

export module carcer.actions.ui.layers:UiShowLayerPopupText;
export import carcer.state;
import sdl2w;
#include "macros.h"

export {

namespace state {

namespace actions {

class UiShowLayerPopupText : public AbstractAction {
  bmin::String title;
  bmin::String text;

  void act() override {
    if (!state) {
      return;
    }
    pushLayerRequest(*state,
                     LayerRequest{.id = LayerId::PopupText, .a = title, .b = text});
  }

public:
  UiShowLayerPopupText(sdl2w::Window* /*_window*/, bmin::String _title, bmin::String _text)
      : title(std::move(_title)), text(std::move(_text)) {}
};

} // namespace actions

} // namespace state

} // export
