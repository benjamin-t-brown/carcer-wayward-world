#pragma once

#include "../Layer.h"
#include <optional>
#include <string_view>
#include <utility>

namespace ui {
class ButtonModal;
}

namespace layers {

class LayerPickUp : public Layer {
  static constexpr int donePressDurationMs = 200;

  bool isClosing = false;
  bool closeEnqueued = false;
  int donePressElapsedMs = 0;
  std::optional<std::pair<int, int>> containerTile;

  void beginCloseWithDonePress();
  ui::ButtonModal* findDoneButton();

public:
  constexpr static std::string_view LAYER_ID = "layer_pick_up";

  explicit LayerPickUp(sdl2w::Window* _window);
  LayerPickUp(sdl2w::Window* _window, int containerX, int containerY);
  virtual ~LayerPickUp() = default;

  void onKeyDown(std::string_view key, int keyCode) override;
  void update(int deltaTime) override;
  void syncCurrentPartyMember();
};

} // namespace layers
