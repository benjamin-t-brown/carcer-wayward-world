module;
#include <cstddef>

export module carcer.actions.world:ClearTownEnemyAiResolving;
export import carcer.state;
#include "macros.h"

export {

namespace state {

namespace actions {

class ClearTownEnemyAiResolving : public AbstractAction {
  void act() override {
    if (!state) {
      return;
    }
    state->world.resolvingTownEnemyAi = false;
  }
};

} // namespace actions

} // namespace state

} // export
