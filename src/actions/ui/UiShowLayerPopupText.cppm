module;
#include <utility>

export module carcer.actions.ui.UiShowLayerPopupText;
export import carcer.state;
import sdl2w;

export {

namespace state {

namespace actions {

class UiShowLayerPopupText : public AbstractAction {
  sdl2w::Window* window;
  bmin::String title;
  bmin::String text;

  void act() override { LayerManagerInterface::showPopupText(window, title, text); }

public:
  UiShowLayerPopupText(sdl2w::Window* _window, bmin::String _title, bmin::String _text)
      : window(_window), title(std::move(_title)), text(std::move(_text)) {}
};

} // namespace actions

} // namespace state

} // export
