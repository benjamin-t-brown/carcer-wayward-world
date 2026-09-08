#include "layers/Layer.h"
#include "layers/LayerManager.h"
#include "state/LayerRequest.h"
#include "state/StateManager.h"
#include <iostream>

namespace {

struct Counts {
  int activated = 0;
  int suspended = 0;
  int deactivated = 0;
  int keyDown = 0;
  int updated = 0;
};

class NonVisualLayer : public layers::Layer {
  Counts& counts;

public:
  NonVisualLayer(Counts& counts, std::string_view id)
      : Layer(nullptr, id), counts(counts) {}

  void onActivate() override { ++counts.activated; }
  void onSuspend() override { ++counts.suspended; }
  void onDeactivate() override { ++counts.deactivated; }
  void onKeyDown(std::string_view, int) override { ++counts.keyDown; }
  void update(int) override { ++counts.updated; }
  // Rendering is deliberately not overridden: non-visual layers need none.
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
              new NonVisualLayer(world, state::layerIdString(request.id)));
        }
        if (request.id == state::LayerId::Inventory) {
          return bmin::UniquePtr<layers::Layer>(
              new NonVisualLayer(inventory, state::layerIdString(request.id)));
        }
        return bmin::UniquePtr<layers::Layer>();
      });

  state::pushLayerRequest(stateManager.getState(),
                          state::LayerRequest{.id = state::LayerId::World});
  manager.update(1);
  ok = expect(manager.getLayerCount() == 1, "world request creates a layer") && ok;
  ok = expect(world.activated == 1, "world activates once") && ok;

  state::pushLayerRequest(stateManager.getState(),
                          state::LayerRequest{.id = state::LayerId::Inventory});
  manager.update(1);
  ok = expect(manager.getLayerCount() == 2, "inventory request creates a layer") && ok;
  ok = expect(world.suspended == 1, "previous layer suspends") && ok;
  ok = expect(inventory.activated == 1, "new front layer activates") && ok;

  manager.handleKeyDown("Enter", 0);
  manager.update(1);
  manager.render(1);
  ok = expect(world.keyDown == 0, "suspended layer receives no event") && ok;
  ok = expect(inventory.keyDown == 1, "active layer receives event") && ok;
  ok = expect(world.updated == 1, "suspended layer does not update") && ok;
  ok = expect(inventory.updated == 2, "active layer updates") && ok;

  state::removeLayerRequest(stateManager.getState(), state::LayerId::Inventory);
  manager.update(1);
  ok = expect(manager.getLayerCount() == 1, "removed request destroys its layer") && ok;
  ok = expect(inventory.deactivated == 1, "removed layer deactivates") && ok;
  ok = expect(world.activated == 2, "previous layer reactivates") && ok;

  state::pushLayerRequest(stateManager.getState(),
                          state::LayerRequest{.id = state::LayerId::Inventory});
  state::pushLayerRequest(stateManager.getState(),
                          state::LayerRequest{.id = state::LayerId::World});
  manager.update(1);
  auto* reorderedWorld =
      manager.getLayerById(state::layerIdString(state::LayerId::World));
  ok = expect(reorderedWorld != nullptr, "request reorder retains world") && ok;
  ok = expect(reorderedWorld && reorderedWorld->getState() == layers::LayerState::ON,
              "last request controls front ordering") && ok;

  return ok ? 0 : 1;
}
