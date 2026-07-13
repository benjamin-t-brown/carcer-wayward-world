#pragma once

#include "../Layer.h"
#include "state/WorldActions.h"
#include "bmin/String.h"
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
  void onKeyUp(std::string_view key, int keyCode) override;
  void update(int deltaTime) override;
  void syncFromState();
  void activateWorldAction(state::WorldActionType worldActionType);
  void cancelWorldActionMode();

private:
  struct HeldMove {
    bmin::String key;
    int dx = 0;
    int dy = 0;
    int heldMs = 0;
    bool repeating = false;
  };

  void processPendingTriggers();
  void attachWorldActionObservers(ui::InGameLayout* inGameLayout);
  void syncWorldActionModeHighlight();
  void syncActionModeCancelButton();
  void enqueuePlayerMove(int dx, int dy);
  void clearHeldMove();
  void updateHeldMoveRepeat(int deltaTime);
  static std::optional<state::WorldActionType>
  worldActionShortcutForKey(std::string_view key);

  std::optional<HeldMove> heldMove;
};

} // namespace layers
