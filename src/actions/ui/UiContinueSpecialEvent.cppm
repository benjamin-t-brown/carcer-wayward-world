module;
#include <cstddef>

export module carcer.actions.ui:UiContinueSpecialEvent;
export import carcer.state;
#include "macros.h"

export {

namespace state {

namespace actions {

// Broadcast-only, see UiSelectSpecialEventChoice.
class UiContinueSpecialEvent : public AbstractAction {};

} // namespace actions

} // namespace state

} // export
