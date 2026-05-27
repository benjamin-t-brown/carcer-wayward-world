#pragma once

#include "state/StateManager.h"
#include "state/actions/ui/UiSetCurrentPartyMemberInventory.hpp"
#include "ui/UiElement.h"

namespace ui {

class ObserverSetCurrentPartyMemberInventory : public ui::UiEventObserver,
                                               public state::StateManagerInterface {
  int partyMemberInventoryIndex;

public:
  explicit ObserverSetCurrentPartyMemberInventory(int _partyMemberInventoryIndex)
      : partyMemberInventoryIndex(_partyMemberInventoryIndex) {}

  void onClick(int mouseX, int mouseY, int button) override {
    LOG(INFO) << "ObserverSetCurrentPartyMemberInventory::onClick index="
              << partyMemberInventoryIndex << LOG_ENDL;
    auto stateManager = getStateManager();
    if (!stateManager) {
      return;
    }
    stateManager->enqueueAction(
        stateManager->getActionData(),
        new state::actions::UiSetCurrentPartyMemberInventory(partyMemberInventoryIndex),
        0);
  }
};

} // namespace ui
