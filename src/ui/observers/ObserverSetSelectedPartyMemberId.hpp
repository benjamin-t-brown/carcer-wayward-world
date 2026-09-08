#pragma once

#include "state/StateManager.h"
#include "actions/navigation/UiSetSelectedPartyMemberId.hpp"
#include "ui/UiElement.h"

namespace ui {

class ObserverSetSelectedPartyMemberId : public ui::UiEventObserver,
                                         public state::StateManagerInterface {
  bmin::String partyMemberId;

public:
  explicit ObserverSetSelectedPartyMemberId(bmin::String _partyMemberId)
      : partyMemberId(std::move(_partyMemberId)) {}

  void onClick(int /*mouseX*/, int /*mouseY*/, int /*button*/) override {
    auto stateManager = getStateManager();
    if (!stateManager) {
      return;
    }
    // Party member switching is locked while combat is active.
    if (stateManager->getState().world.combat.active) {
      return;
    }
    LOG(INFO) << "ObserverSetSelectedPartyMemberId::onClick id=" << partyMemberId
              << LOG_ENDL;
    stateManager->enqueueAction(state::makeAction<state::actions::UiSetSelectedPartyMemberId>(partyMemberId),
        0);
  }
};

} // namespace ui
