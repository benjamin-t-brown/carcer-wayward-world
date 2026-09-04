module;
#include <utility>
#include <cstddef>

export module carcer.actions.world:ModifyPartyMemberHp;
export import carcer.state;
import carcer.model.templates;
import bmin.string_interop;
#include "macros.h"

export {

namespace state {

namespace actions {

// Applies town-mode party HP change (victim need not be on the active map).
class ModifyPartyMemberHp : public AbstractAction {
  bmin::String instanceId;
  int delta = 0;

  void act() override {
    if (!state) {
      return;
    }
    model::modifyPartyMemberHp(state->player, instanceId, delta);
  }

public:
  ModifyPartyMemberHp(bmin::String _instanceId, int _delta)
      : instanceId(std::move(_instanceId)), delta(_delta) {}
};

} // namespace actions

} // namespace state

} // export
