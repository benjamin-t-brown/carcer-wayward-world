export module carcer.actions.ui.UiShowLayerPickupContext;
export import carcer.state;
import sdl2w;

export {

namespace state {

namespace actions {

class UiShowLayerPickupContext : public AbstractAction {
  sdl2w::Window* window;
  model::ItemInstance item;
  void act() override { LayerManagerInterface::showPickupContext(window, item); }

public:
  UiShowLayerPickupContext(sdl2w::Window* _window, const model::ItemInstance& item)
      : window(_window), item(item) {}
};

} // namespace actions

} // namespace state

} // export
