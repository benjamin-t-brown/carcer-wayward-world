#pragma once

#include "bmin/String.h"
#include "model/instances/Player.h"
#include "state/AbstractAction.hpp"
#include "state/State.hpp"

namespace state::actions {

// Applies town-mode party HP change (victim need not be on the active map).
class ModifyPartyMemberHp : public AbstractAction {
  ActionEvent getEvent() const override {
    return ActionEvent::ModifyPartyMemberHp;
  }

  bmin::String instanceId;
  int delta = 0;

  void act() override {
    if (state) {
      model::modifyPartyMemberHp(state->player, instanceId, delta);
    }
  }

public:
  ModifyPartyMemberHp(bmin::String instanceId, int delta)
      : instanceId(std::move(instanceId)), delta(delta) {}
};

} // namespace state::actions
