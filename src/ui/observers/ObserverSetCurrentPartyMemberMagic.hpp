#pragma once

#include "state/StateManager.h"
#include "actions/navigation/UiSetCurrentPartyMemberMagic.hpp"
#include "ui/UiElement.h"

namespace ui {

class ObserverSetCurrentPartyMemberMagic : public ui::UiEventObserver,
                                           public state::StateManagerInterface {
  int partyMemberMagicIndex;

public:
  explicit ObserverSetCurrentPartyMemberMagic(int _partyMemberMagicIndex)
      : partyMemberMagicIndex(_partyMemberMagicIndex) {}

  void onClick(int mouseX, int mouseY, int button) override {
    LOG(INFO) << "ObserverSetCurrentPartyMemberMagic::onClick index="
              << partyMemberMagicIndex << LOG_ENDL;
    auto stateManager = getStateManager();
    if (!stateManager) {
      return;
    }
    stateManager->enqueueAction(
        stateManager->getActionData(),
        new state::actions::UiSetCurrentPartyMemberMagic(partyMemberMagicIndex),
        0);
  }
};

} // namespace ui
