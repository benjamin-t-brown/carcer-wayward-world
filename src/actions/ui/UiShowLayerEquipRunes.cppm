export module carcer.actions.ui.UiShowLayerEquipRunes;
export import carcer.state;
import sdl2w;

export {

namespace state {

namespace actions {

class UiShowLayerEquipRunes : public AbstractAction {
  sdl2w::Window* window;
  bmin::String characterPlayerId;

  void act() override {
    LayerManagerInterface::showEquipRunes(window, characterPlayerId);
  }

public:
  UiShowLayerEquipRunes(sdl2w::Window* _window, const bmin::String& _characterPlayerId)
      : window(_window), characterPlayerId(_characterPlayerId) {}
};

} // namespace actions

} // namespace state

} // export
