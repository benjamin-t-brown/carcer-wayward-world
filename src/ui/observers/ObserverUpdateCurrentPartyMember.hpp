#pragma once

#include "state/StateManager.h"
#include "state/actions/ui/UiSetCurrentPartyMember.hpp"
#include "ui/UiElement.h"

namespace ui {

class ObserverUpdateCurrentPartyMember : public ui::UiEventObserver,
                                         public state::StateManagerInterface {
  int directionDelta;

public:
  ObserverUpdateCurrentPartyMember(int /*_partyMemberIndex*/, int _directionDelta)
      : directionDelta(_directionDelta) {}

  void onClick(int mouseX, int mouseY, int button) override {
    LOG(INFO) << "ObserverUpdateCurrentPartyMember::onClick delta=" << directionDelta
              << LOG_ENDL;
    auto stateManager = getStateManager();
    if (!stateManager) {
      return;
    }

    auto& player = stateManager->getState().player;
    const int partySize = static_cast<int>(player.party.size());
    if (partySize == 0) {
      return;
    }

    int nextIndex = player.currentPartyMemberIndex + directionDelta;
    if (nextIndex < 0) {
      nextIndex = partySize - 1;
    } else if (nextIndex >= partySize) {
      nextIndex = 0;
    }

    stateManager->enqueueAction(stateManager->getActionData(),
                                new state::actions::UiSetCurrentPartyMember(nextIndex),
                                0);
  }
};

} // namespace ui
