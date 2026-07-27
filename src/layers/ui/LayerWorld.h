#pragma once

#include "../Layer.h"
#include "bmin/String.h"
#include "model/templates/UtilityTypes.h"
#include "state/WorldActions.h"
#include <string_view>

namespace ui {
class InGameLayout;
}

namespace layers {

class LayerWorld : public Layer {
private:
  // void processPendingTriggers();
  void attachWorldActionObservers(ui::InGameLayout* inGameLayout);
  void syncWorldActionModeHighlight();
  void syncActionModeCancelButton();
  void syncCombatTitleBar();
  void updateHeldMoveRepeat(int deltaTime);
  void confirmWorldActionAim(int tileX, int tileY);
  void updateAimFromMouse(int x, int y);
  float mapScale = 1.f;
  bool hasLastMousePos = false;
  int lastMouseX = 0;
  int lastMouseY = 0;

  void alignMapView();
  void fillWorldActionTypes(model::TurnMode turnMode,
                            bmin::DynArray<state::WorldActionType>& dest);

public:
  constexpr static std::string_view LAYER_ID = "layer_world";

  explicit LayerWorld(sdl2w::Window* _window);
  virtual ~LayerWorld() = default;

  void onKeyDown(std::string_view key, int keyCode) override;
  void onKeyUp(std::string_view key, int keyCode) override;
  void onMouseDown(int x, int y, int button) override;
  void onMouseHover(int x, int y) override;
  void syncFromState();
  void setMapScale(float scale);
  void update(int deltaTime) override;
  void render(int deltaTime) override;
};

} // namespace layers
