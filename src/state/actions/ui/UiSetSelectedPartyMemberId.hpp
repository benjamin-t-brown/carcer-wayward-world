#pragma once

#include "model/instances/Player.h"
#include "state/AbstractAction.h"
#include "state/State.h"

namespace state {

namespace actions {

/** UI selection only — does not affect map movement / party avatar. */
class UiSetSelectedPartyMemberId : public AbstractAction {
  bmin::String partyMemberId;

  void act() override {
    auto& localState = *state;
    if (model::playerFindPartyMemberIndexById(localState.player, partyMemberId) < 0) {
      LOG(WARN) << "UiSetSelectedPartyMemberId::act: party member id not found "
                << partyMemberId << LOG_ENDL;
      return;
    }

    localState.uiState.selectedPartyMemberId = partyMemberId;
  }

public:
  explicit UiSetSelectedPartyMemberId(bmin::String _partyMemberId)
      : partyMemberId(std::move(_partyMemberId)) {}
};

} // namespace actions

} // namespace state
