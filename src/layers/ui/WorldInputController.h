#pragma once

#include "state/DatabaseInterface.h"
#include "state/StateManagerInterface.h"
#include <string_view>

namespace layers {

class UiLayer;

/**
 * Turns raw keyboard/mouse events into world State intents (movement, held-move
 * repeats, action-mode aim). Owns the transient mouse-position tracking used to
 * distinguish real cursor movement from a stationary cursor over the map.
 *
 * State and database come from the inherited singleton interfaces — the same
 * instances LayerWorld sees. The per-layer handles that are not singletons
 * (window, owned UI elements) come from `owner`, whose getWindow/getUiElement
 * accessors are public.
 */
class WorldInputController : public state::StateManagerInterface,
                             public state::DatabaseInterface {
public:
  explicit WorldInputController(UiLayer& owner) : owner(owner) {}

  void onKeyDown(std::string_view key, int keyCode);
  void onKeyUp(std::string_view key, int keyCode);
  // Returns true if the click was consumed (action-mode aim confirm); the layer
  // should skip base-class dispatch when so.
  bool onMouseDown(int x, int y, int button);

  // Polled each frame from LayerWorld::update.
  void updateAimFromMouse(int x, int y);
  void updateHeldMoveRepeat(int deltaTime);

private:
  UiLayer& owner;

  bool hasLastMousePos = false;
  int lastMouseX = 0;
  int lastMouseY = 0;
};

} // namespace layers
