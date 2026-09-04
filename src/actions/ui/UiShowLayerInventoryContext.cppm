export module carcer.actions.ui.UiShowLayerInventoryContext;
export import carcer.state;
import sdl2w;

export {

namespace state {

namespace actions {

class UiShowLayerInventoryContext : public AbstractAction {
  sdl2w::Window* window;
  bmin::String itemName;
  bmin::String itemId;
  void act() override {
    LayerManagerInterface::showInventoryContext(window, itemId, itemName);
  }

public:
  UiShowLayerInventoryContext(sdl2w::Window* _window,
                              bmin::String itemName,
                              bmin::String itemId)
      : window(_window), itemName(itemName), itemId(itemId) {}
};

} // namespace actions

} // namespace state

} // export
