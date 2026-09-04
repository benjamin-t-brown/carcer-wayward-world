export module carcer.actions.ui.UiShowLayerSpellCast;
export import carcer.state;
import sdl2w;

export {

namespace state {

namespace actions {

class UiShowLayerSpellCast : public AbstractAction {
  sdl2w::Window* window;
  bmin::String chId;

  void act() override { LayerManagerInterface::showSpellCast(window, chId); }

public:
  explicit UiShowLayerSpellCast(sdl2w::Window* _window, const bmin::String& chId)
      : window(_window), chId(chId) {}
};

} // namespace actions

} // namespace state

} // export
