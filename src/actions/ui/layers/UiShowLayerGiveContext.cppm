module;
#include <utility>
#include <cstddef>

export module carcer.actions.ui.layers:UiShowLayerGiveContext;
export import carcer.state;
import sdl2w;
#include "macros.h"

export {

namespace state {

namespace actions {

class UiShowLayerGiveContext : public AbstractAction {
  bmin::String fromCharacterPlayerId;
  bmin::String itemId;

  void act() override {
    if (!state) {
      return;
    }
    pushLayerRequest(*state,
                     LayerRequest{.id = LayerId::GiveContext,
                                  .a = fromCharacterPlayerId,
                                  .b = itemId});
  }

public:
  UiShowLayerGiveContext(sdl2w::Window* /*_window*/,
                         bmin::String _fromCharacterPlayerId,
                         bmin::String _itemId)
      : fromCharacterPlayerId(std::move(_fromCharacterPlayerId)),
        itemId(std::move(_itemId)) {}
};

} // namespace actions

} // namespace state

} // export
