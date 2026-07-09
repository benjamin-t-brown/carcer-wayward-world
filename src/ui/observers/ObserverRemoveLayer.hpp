#pragma once

#include "lib/StringUtil.h"
#include "state/StateManager.h"
#include "state/actions/ui/UiRemoveLayer.hpp"
#include "ui/UiElement.h"

namespace ui {
class ObserverRemoveLayer : public ui::UiEventObserver,
                            public state::StateManagerInterface {
  bmin::String layerId;

public:
  ObserverRemoveLayer(const bmin::String& _layerId) : layerId(_layerId) {}
  ObserverRemoveLayer(std::string_view _layerId) : layerId(strutil::fromStringView(_layerId)) {}

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
