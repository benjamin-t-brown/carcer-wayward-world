#pragma once

#include "bmin/String.h"
#include "state/StateManager.h"
#include "actions/navigation/UiToggleManaSlotRune.hpp"
#include "ui/UiElement.h"

namespace ui {

class ObserverToggleManaSlotRune : public ui::UiEventObserver,
                                   public state::StateManagerInterface {
  bmin::String characterPlayerId;
  size_t slotIndex = 0;

public:
  ObserverToggleManaSlotRune(const bmin::String& _characterPlayerId, size_t _slotIndex)
      : characterPlayerId(_characterPlayerId), slotIndex(_slotIndex) {}

  void onClick(int /*mouseX*/, int /*mouseY*/, int /*button*/) override {
    LOG(INFO) << "ObserverToggleManaSlotRune::onClick slot=" << slotIndex << LOG_ENDL;
    auto stateManager = getStateManager();
    if (!stateManager) {
      return;
    }
    stateManager->enqueueAction(
        stateManager->getActionData(),
        new state::actions::UiToggleManaSlotRune(characterPlayerId, slotIndex),
        0);
  }
};

} // namespace ui
