#pragma once

#include "../Layer.h"
#include "state/WorldActions.h"
#include <optional>
#include <string_view>

namespace ui {
class InGameLayout;
}

namespace layers {

class LayerWorld : public Layer {
public:
  constexpr static std::string_view LAYER_ID = "layer_world";

  explicit LayerWorld(sdl2w::Window* _window);
  virtual ~LayerWorld() = default;

  void onKeyDown(std::string_view key, int keyCode) override;
  void update(int deltaTime) override;
  void syncFromState();
  void activateWorldAction(state::WorldActionType worldActionType);

private:
  void processPendingTriggers();
  void attachWorldActionObservers(ui::InGameLayout* inGameLayout);
  void syncWorldActionModeHighlight();
  static std::optional<state::WorldActionType>
  worldActionShortcutForKey(std::string_view key);
};

} // namespace layers
