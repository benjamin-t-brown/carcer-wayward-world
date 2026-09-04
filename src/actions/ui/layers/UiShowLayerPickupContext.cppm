module;
#include <cstddef>

export module carcer.actions.ui.layers:UiShowLayerPickupContext;
export import carcer.state;
import sdl2w;
import carcer.model.instances;
#include "macros.h"

export {

namespace state {

namespace actions {

class UiShowLayerPickupContext : public AbstractAction {
  bmin::String itemId;
  void act() override {
    if (!state) {
      return;
    }
    // Reconciler re-resolves the full ItemInstance from state by id.
    pushLayerRequest(*state, LayerRequest{.id = LayerId::PickUpContext, .a = itemId});
  }

public:
  UiShowLayerPickupContext(sdl2w::Window* /*_window*/, const model::ItemInstance& item)
      : itemId(item.id) {}
};

} // namespace actions

} // namespace state

} // export
