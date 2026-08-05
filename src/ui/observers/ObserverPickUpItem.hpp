#pragma once

#include "model/instances/ItemInstance.h"
#include "state/StateManager.h"
#include "state/actions/ui/UiPickUpItem.hpp"
#include "ui/UiElement.h"

namespace ui {

class ObserverPickUpItem : public ui::UiEventObserver,
                           public state::StateManagerInterface {
  bmin::String itemId;

public:
  explicit ObserverPickUpItem(const model::ItemInstance& item) : itemId(item.id) {}

  void onClick(int /*mouseX*/, int /*mouseY*/, int /*button*/) override {
    LOG(INFO) << "ObserverPickUpItem::onClick id=" << itemId << LOG_ENDL;
    auto stateManager = getStateManager();
    if (!stateManager) {
      return;
    }
    stateManager->enqueueAction(stateManager->getActionData(),
                                new state::actions::UiPickUpItem(itemId),
                                0);
  }
};

} // namespace ui
