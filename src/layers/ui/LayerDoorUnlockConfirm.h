#pragma once

#include "bmin/String.h"
#include "game/map/DoorLock.h"
#include "layers/UiLayer.h"
#include "ui/KeyboardPressFlash.h"
#include "ui/elements/buttons/ButtonModal.h"
#include <functional>

namespace layers {

class LayerDoorUnlockConfirm : public UiLayer {
public:
  constexpr static std::string_view LAYER_ID = "layer_door_unlock_confirm";
  explicit LayerDoorUnlockConfirm(sdl2w::Window* _window, int worldX, int worldY);
  ~LayerDoorUnlockConfirm() override = default;

  void onKeyDown(std::string_view key, int keyCode) override;
  void update(int deltaTime) override;
  void render(int deltaTime) override;

private:
  int worldX = 0;
  int worldY = 0;
  bool dismissOnly = false;
  ui::KeyboardPressFlash keyboardFlash;

  static bmin::String messageForBump(const game::ClosedDoorBumpInfo& bump);
  static bool isConfirmOutcome(game::ClosedDoorBumpOutcome outcome);

  ui::ButtonModal* buttonAtIndex(int index);
  void beginKeyboardDismissPress();
  void beginKeyboardConfirmPress();
  void beginKeyboardButtonPress(int buttonIndex, std::function<void()> onComplete);
  void enqueueDismiss();
  void enqueueConfirm();
};

} // namespace layers
