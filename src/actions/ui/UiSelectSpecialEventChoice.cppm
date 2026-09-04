module;
#include <cstddef>

export module carcer.actions.ui:UiSelectSpecialEventChoice;
export import carcer.state;
#include "macros.h"

export {

namespace state {

namespace actions {

// Broadcast-only: carries the payload for LayerSpecialEvent's own
// subscribeAction<> to react to. UI never mutates a special event's talk
// history / runner state directly.
class UiSelectSpecialEventChoice : public AbstractAction {
public:
  int choiceIndex = -1;
  explicit UiSelectSpecialEventChoice(int _choiceIndex) : choiceIndex(_choiceIndex) {}
};

} // namespace actions

} // namespace state

} // export
