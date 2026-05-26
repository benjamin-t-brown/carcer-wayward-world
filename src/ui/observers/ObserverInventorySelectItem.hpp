#pragma once

#include "ui/UiElement.h"

namespace ui {
class ObserverInventorySelectItem : public ui::UiEventObserver,
                                    public state::StateManagerInterface {

  std::string itemName;
  std::string itemId;

public:
  ObserverInventorySelectItem(std::string_view itemName, std::string_view itemId)
      : itemName(itemName), itemId(itemId) {}

  void onClick(int mouseX, int mouseY, int button) override {
    LOG(INFO) << "ObserverInventorySelectItem::onClick " << itemName << " " << itemId
              << LOG_ENDL;
    auto stateManager = getStateManager();
    if (!stateManager) {
      return;
    }
  }
};
} // namespace ui
