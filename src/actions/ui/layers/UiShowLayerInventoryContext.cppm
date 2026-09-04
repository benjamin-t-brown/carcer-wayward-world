module;
#include <cstddef>

export module carcer.actions.ui.layers:UiShowLayerInventoryContext;
export import carcer.state;
import sdl2w;
#include "macros.h"

export {

namespace state {

namespace actions {

class UiShowLayerInventoryContext : public AbstractAction {
  bmin::String itemName;
  bmin::String itemId;
  void act() override {
    if (!state) {
      return;
    }
    pushLayerRequest(
        *state,
        LayerRequest{.id = LayerId::InventoryContext, .a = itemId, .b = itemName});
  }

public:
  UiShowLayerInventoryContext(sdl2w::Window* /*_window*/,
                              bmin::String itemName,
                              bmin::String itemId)
      : itemName(itemName), itemId(itemId) {}
};

} // namespace actions

} // namespace state

} // export
