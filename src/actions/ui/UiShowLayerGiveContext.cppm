module;
#include <utility>

export module carcer.actions.ui.UiShowLayerGiveContext;
export import carcer.state;
import sdl2w;

export {

namespace state {

namespace actions {

class UiShowLayerGiveContext : public AbstractAction {
  sdl2w::Window* window;
  bmin::String fromCharacterPlayerId;
  bmin::String itemId;

  void act() override {
    LayerManagerInterface::showGiveContext(window, fromCharacterPlayerId, itemId);
  }

public:
  UiShowLayerGiveContext(sdl2w::Window* _window,
                         bmin::String _fromCharacterPlayerId,
                         bmin::String _itemId)
      : window(_window),
        fromCharacterPlayerId(std::move(_fromCharacterPlayerId)),
        itemId(std::move(_itemId)) {}
};

} // namespace actions

} // namespace state

} // export
