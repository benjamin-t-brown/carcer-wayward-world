export module carcer.actions.ui.UiShowLayerInventory;
export import carcer.state;
import sdl2w;

export {

namespace state {

namespace actions {

class UiShowLayerInventory : public AbstractAction {
  sdl2w::Window* window;

  void act() override {
    if (!state) {
      return;
    }
    auto& player = state->player;
    const int selectedIndex = model::playerFindPartyMemberIndexById(
        player, state->uiState.selectedPartyMemberId);
    player.currentPartyMemberInventoryIndex = selectedIndex >= 0 ? selectedIndex : 0;
    LayerManagerInterface::showInventory(window);
  }

public:
  explicit UiShowLayerInventory(sdl2w::Window* _window) : window(_window) {}
};

} // namespace actions

} // namespace state

} // export
