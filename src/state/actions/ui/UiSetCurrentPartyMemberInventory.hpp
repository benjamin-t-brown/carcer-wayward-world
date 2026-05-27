#pragma once

#include "state/AbstractAction.h"
#include "state/State.h"

namespace state {

namespace actions {

class UiSetCurrentPartyMemberInventory : public AbstractAction {
  int partyMemberInventoryIndex = 0;

  void act() override {
    auto& localState = *state;

    int nextIndex = partyMemberInventoryIndex;
    if (partyMemberInventoryIndex < 0 ||
        partyMemberInventoryIndex >= static_cast<int>(localState.player.party.size())) {
      nextIndex = 0;
      LOG(WARN) << "UiSetCurrentPartyMemberInventory::act: index out of range "
                << partyMemberInventoryIndex << " "
                << localState.player.party.size() << LOG_ENDL;
    }
    localState.player.currentPartyMemberInventoryIndex = nextIndex;
  }

public:
  explicit UiSetCurrentPartyMemberInventory(int _partyMemberInventoryIndex)
      : partyMemberInventoryIndex(_partyMemberInventoryIndex) {}
};

} // namespace actions

} // namespace state
