#pragma once

#include "state/StateManager.h"
#include "state/actions/ui/UiRemoveLayer.hpp"
#include "ui/UiElement.h"

namespace ui {
class ObserverRemoveLayer : public ui::UiEventObserver,
                            public state::StateManagerInterface {
  std::string layerId;

public:
  ObserverRemoveLayer(std::string_view layerId) : layerId(layerId) {}

  void onClick(int mouseX, int mouseY, int button) override {
    LOG(INFO) << "ObserverRemoveLayer::onClick " << layerId << LOG_ENDL;
    auto stateManager = getStateManager();
    if (!stateManager || layerId.empty()) {
      return;
    }
    stateManager->enqueueAction(
        stateManager->getActionData(), new state::actions::UiRemoveLayer(layerId), 0);
  }
};
} // namespace ui
