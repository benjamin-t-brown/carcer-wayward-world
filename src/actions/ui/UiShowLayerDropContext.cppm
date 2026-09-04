module;
#include <utility>

export module carcer.actions.ui.UiShowLayerDropContext;
export import carcer.state;
import sdl2w;

export {

namespace state {

namespace actions {

class UiShowLayerDropContext : public AbstractAction {
  sdl2w::Window* window;
  bmin::String characterPlayerId;
  bmin::String itemId;

  void act() override {
    LayerManagerInterface::showDropConfirm(window, characterPlayerId, itemId);
  }

public:
  UiShowLayerDropContext(sdl2w::Window* _window,
                         bmin::String _characterPlayerId,
                         bmin::String _itemId)
      : window(_window),
        characterPlayerId(std::move(_characterPlayerId)),
        itemId(std::move(_itemId)) {}
};

} // namespace actions

} // namespace state

} // export
