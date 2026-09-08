// Characterization test for LayerManager input routing and render order.
//
// Documents current behavior that Phase 5 (authoritative stack + consumed
// event routing) must preserve for the active layer:
//   - mouse-down, mouse-up, wheel, key-down, and key-up reach the front ON
//     layer and not a SUSPENDED layer beneath it;
//   - both ON and SUSPENDED layers render.
#include "layers/Layer.h"
#include "layers/LayerManager.h"
#include "state/LayerRequest.h"
#include "state/StateManager.h"
#include <iostream>

namespace {

struct Counts {
  int mouseDown = 0;
  int mouseUp = 0;
  int wheel = 0;
  int keyDown = 0;
  int keyUp = 0;
  int rendered = 0;
};

class CountingLayer : public layers::Layer {
  Counts& counts;

public:
  CountingLayer(Counts& counts, std::string_view id)
      : Layer(nullptr, id), counts(counts) {}

  void onMouseDown(int, int, int) override { ++counts.mouseDown; }
  void onMouseUp(int, int, int) override { ++counts.mouseUp; }
  void onMouseWheel(int, int, int) override { ++counts.wheel; }
  void onKeyDown(std::string_view, int) override { ++counts.keyDown; }
  void onKeyUp(std::string_view, int) override { ++counts.keyUp; }
  void render(int) override { ++counts.rendered; }
};

bool expect(bool condition, const char* message) {
  if (!condition) {
    std::cerr << "FAIL: " << message << '\n';
  }
  return condition;
}

} // namespace

int main() {
  bool ok = true;
  state::StateManager stateManager;
  Counts world;
  Counts inventory;

  layers::LayerManager manager(
      nullptr,
      [&](const state::LayerRequest& request) -> bmin::UniquePtr<layers::Layer> {
        if (request.id == state::LayerId::World) {
          return bmin::UniquePtr<layers::Layer>(
              new CountingLayer(world, state::layerIdString(request.id)));
        }
        if (request.id == state::LayerId::Inventory) {
          return bmin::UniquePtr<layers::Layer>(
              new CountingLayer(inventory, state::layerIdString(request.id)));
        }
        return bmin::UniquePtr<layers::Layer>();
      });

  // World layer active.
  state::pushLayerRequest(stateManager.getState(),
                          state::LayerRequest{.id = state::LayerId::World});
  manager.update(1);

  // Overlay inventory: suspends world, activates inventory.
  state::pushLayerRequest(stateManager.getState(),
                          state::LayerRequest{.id = state::LayerId::Inventory});
  manager.update(1);

  manager.handleMouseDown(1, 2, 0);
  manager.handleMouseUp(1, 2, 0);
  manager.handleMouseWheel(0, 0, 1);
  manager.handleKeyDown("Enter", 0);
  manager.handleKeyUp("Enter", 0);

  ok = expect(inventory.mouseDown == 1, "front layer receives mouse-down") && ok;
  ok = expect(inventory.mouseUp == 1, "front layer receives mouse-up") && ok;
  ok = expect(inventory.wheel == 1, "front layer receives wheel") && ok;
  ok = expect(inventory.keyDown == 1, "front layer receives key-down") && ok;
  ok = expect(inventory.keyUp == 1, "front layer receives key-up") && ok;

  ok = expect(world.mouseDown == 0, "suspended layer receives no mouse-down") && ok;
  ok = expect(world.mouseUp == 0, "suspended layer receives no mouse-up") && ok;
  ok = expect(world.wheel == 0, "suspended layer receives no wheel") && ok;
  ok = expect(world.keyDown == 0, "suspended layer receives no key-down") && ok;
  ok = expect(world.keyUp == 0, "suspended layer receives no key-up") && ok;

  // Both ON and SUSPENDED layers render.
  manager.render(1);
  ok = expect(inventory.rendered == 1, "active layer renders") && ok;
  ok = expect(world.rendered == 1, "suspended layer still renders") && ok;

  return ok ? 0 : 1;
}
