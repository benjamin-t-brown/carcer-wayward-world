#pragma once

#include "../Layer.h"
#include "bmin/String.h"
#include "model/templates/UtilityTypes.h"
#include "state/WorldActions.h"
#include <optional>
#include <string_view>

namespace ui {
class InGameLayout;
}

namespace layers {

class LayerWorld : public Layer {
private:
  struct HeldMove {
    bool isActive = false;
    bmin::String key;
    int dx = 0;
    int dy = 0;
    model::TimerStruct initialDelay = model::TimerStruct(300);
    model::TimerStruct moveDelay = model::TimerStruct(50);
  };
  HeldMove heldMove;

  struct MoveDelta {
    int dx = 0;
    int dy = 0;
  };

  void processPendingTriggers();
  void attachWorldActionObservers(ui::InGameLayout* inGameLayout);
  void syncWorldActionModeHighlight();
  void syncActionModeCancelButton();
  void updateHeldMoveRepeat(int deltaTime);
  static std::optional<state::WorldActionType>
  /**/ worldActionShortcutForKey(std::string_view key);
  void alignMapView();
  void fillWorldActionTypes(model::TurnMode turnMode,
                            bmin::DynArray<state::WorldActionType>& dest);

public:
  constexpr static std::string_view LAYER_ID = "layer_world";

  explicit LayerWorld(sdl2w::Window* _window);
  virtual ~LayerWorld() = default;

  void onKeyDown(std::string_view key, int keyCode) override;
  void onKeyUp(std::string_view key, int keyCode) override;
  void syncFromState();
  void activateWorldAction(state::WorldActionType worldActionType);
  void cancelWorldActionMode();
  std::optional<MoveDelta> getMoveDeltaForKey(std::string_view key);
  void update(int deltaTime) override;
};

} // namespace layers
