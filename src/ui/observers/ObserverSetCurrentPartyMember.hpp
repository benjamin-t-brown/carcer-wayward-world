#pragma once

#include "state/StateManager.h"
#include "actions/navigation/UiSetCurrentPartyMember.hpp"
#include "ui/UiElement.h"

namespace ui {

class ObserverSetCurrentPartyMember : public ui::UiEventObserver,
                                      public state::StateManagerInterface {
  int partyMemberIndex;

public:
  explicit ObserverSetCurrentPartyMember(int _partyMemberIndex)
      : partyMemberIndex(_partyMemberIndex) {}

  void onClick(int mouseX, int mouseY, int button) override {
    LOG(INFO) << "ObserverSetCurrentPartyMember::onClick index=" << partyMemberIndex
              << LOG_ENDL;
    auto stateManager = getStateManager();
    if (!stateManager) {
      return;
    }
    stateManager->enqueueAction(state::makeAction<state::actions::UiSetCurrentPartyMember>(partyMemberIndex),
                                0);
  }
};

} // namespace ui
