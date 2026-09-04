module;
#include <utility>
#include <cstddef>

export module carcer.actions.ui.layers:UiShowLayerDropContext;
export import carcer.state;
import sdl2w;
#include "macros.h"

export {

namespace state {

namespace actions {

class UiShowLayerDropContext : public AbstractAction {
  bmin::String characterPlayerId;
  bmin::String itemId;

  void act() override {
    if (!state) {
      return;
    }
    pushLayerRequest(*state,
                     LayerRequest{.id = LayerId::DropConfirm,
                                  .a = characterPlayerId,
                                  .b = itemId});
  }

public:
  UiShowLayerDropContext(sdl2w::Window* /*_window*/,
                         bmin::String _characterPlayerId,
                         bmin::String _itemId)
      : characterPlayerId(std::move(_characterPlayerId)),
        itemId(std::move(_itemId)) {}
};

} // namespace actions

} // namespace state

} // export
