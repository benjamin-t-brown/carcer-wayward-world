#pragma once

#include "ui/UiElement.h"

namespace ui {
class ObserverPickUpItem : public ui::UiEventObserver,
                           public state::StateManagerInterface {

  model::ItemInstance item;

public:
  ObserverPickUpItem(const model::ItemInstance& item) : item(item) {}

  void onClick(int mouseX, int mouseY, int button) override {
    LOG(INFO) << "ObserverPickUpItem::onClick " << item.itemName << " " << item.id
              << LOG_ENDL;
    auto stateManager = getStateManager();
    if (!stateManager) {
      return;
    }
  }
};
} // namespace ui
