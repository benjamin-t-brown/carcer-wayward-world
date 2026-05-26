#pragma once

#include "state/AbstractAction.h"
#include "state/State.h"

namespace state {

namespace actions {

class UiSetCurrentPartyMember : public AbstractAction {
  int partyMemberIndex = 0;
  void act() override {
    auto& localState = *state;

    int nextPartyMemberIndex = partyMemberIndex;
    if (partyMemberIndex < 0 ||
        partyMemberIndex >= static_cast<int>(localState.player.party.size())) {
      nextPartyMemberIndex = 0;
      LOG(WARN) << "UiSetCurrentPartyMember::act: partyMemberIndex is out of range " +
                       std::to_string(partyMemberIndex) + " " +
                       std::to_string(localState.player.party.size())
                << LOG_ENDL;
    }
    localState.player.currentPartyMemberIndex = nextPartyMemberIndex;
  }

public:
  UiSetCurrentPartyMember(int _partyMemberIndex) : partyMemberIndex(_partyMemberIndex) {}
};

} // namespace actions

} // namespace state
