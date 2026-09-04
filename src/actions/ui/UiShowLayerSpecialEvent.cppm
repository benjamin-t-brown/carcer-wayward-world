module;
#include <utility>

export module carcer.actions.ui.UiShowLayerSpecialEvent;
export import carcer.state;
import sdl2w;

export {

namespace state {

namespace actions {

class UiShowLayerSpecialEvent : public AbstractAction {
  sdl2w::Window* window;
  bmin::String eventId;

  void act() override {
    if (!state) {
      return;
    }
    LayerManagerInterface::showSpecialEvent(window, eventId, *state);
  }

public:
  UiShowLayerSpecialEvent(sdl2w::Window* _window, bmin::String _eventId)
      : window(_window), eventId(std::move(_eventId)) {}
};

} // namespace actions

} // namespace state

} // export
