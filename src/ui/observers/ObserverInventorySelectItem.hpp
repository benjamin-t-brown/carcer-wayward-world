#pragma once

#include "state/StateManager.h"
#include "state/actions/ui/UiToggleEquipInventoryItem.hpp"
#include "ui/UiElement.h"

namespace ui {
class ObserverInventorySelectItem : public ui::UiEventObserver,
                                    public state::StateManagerInterface {

  std::string characterPlayerId;
  std::string itemId;

public:
  ObserverInventorySelectItem(std::string_view _characterPlayerId, std::string_view _itemId)
      : characterPlayerId(_characterPlayerId), itemId(_itemId) {}

  void onClick(int mouseX, int mouseY, int button) override {
    LOG(INFO) << "ObserverInventorySelectItem::onClick " << itemId << LOG_ENDL;
    auto stateManager = getStateManager();
    if (!stateManager) {
      return;
    }
    stateManager->enqueueAction(
        stateManager->getActionData(),
        new state::actions::UiToggleEquipInventoryItem(characterPlayerId, itemId),
        0);
  }
};
} // namespace ui
