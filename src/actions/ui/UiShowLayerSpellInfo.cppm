export module carcer.actions.ui.UiShowLayerSpellInfo;
export import carcer.state;
import sdl2w;

export {

namespace state {

namespace actions {

class UiShowLayerSpellInfo : public AbstractAction {
  sdl2w::Window* window;
  bmin::String spellName;

  void act() override { LayerManagerInterface::showSpellInfo(window, spellName); }

public:
  UiShowLayerSpellInfo(sdl2w::Window* _window, const bmin::String& _spellName)
      : window(_window), spellName(_spellName) {}
};

} // namespace actions

} // namespace state

} // export
