export module carcer.actions.ui.UiShowLayerMagic;
export import carcer.state;
import sdl2w;

export {

namespace state {

namespace actions {

class UiShowLayerMagic : public AbstractAction {
  sdl2w::Window* window;

  void act() override {
    if (!state) {
      return;
    }
    auto& player = state->player;
    const int selectedIndex = model::playerFindPartyMemberIndexById(
        player, state->uiState.selectedPartyMemberId);
    player.currentPartyMemberMagicIndex = selectedIndex >= 0 ? selectedIndex : 0;
    LayerManagerInterface::showMagic(window);
  }

public:
  explicit UiShowLayerMagic(sdl2w::Window* _window) : window(_window) {}
};

} // namespace actions

} // namespace state

} // export
