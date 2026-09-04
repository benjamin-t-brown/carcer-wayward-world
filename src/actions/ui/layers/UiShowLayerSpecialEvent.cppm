module;
#include <utility>
#include <cstddef>

export module carcer.actions.ui.layers:UiShowLayerSpecialEvent;
export import carcer.state;
import sdl2w;
#include "macros.h"

export {

namespace state {

namespace actions {

class UiShowLayerSpecialEvent : public AbstractAction {
  bmin::String eventId;

  void act() override {
    if (!state) {
      return;
    }
    // Reconciler looks up the GameEvent (+ related events, storage) from
    // eventId via the database.
    pushLayerRequest(*state, LayerRequest{.id = LayerId::SpecialEvent, .a = eventId});
  }

public:
  UiShowLayerSpecialEvent(sdl2w::Window* /*_window*/, bmin::String _eventId)
      : eventId(std::move(_eventId)) {}
};

} // namespace actions

} // namespace state

} // export
