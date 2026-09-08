#pragma once

#include "state/StateManager.h"
#include "actions/navigation/UiRemoveLayer.hpp"
#include "ui/UiElement.h"

namespace ui {
class ObserverRemoveLayer : public ui::UiEventObserver,
                            public state::StateManagerInterface {
  std::optional<state::LayerId> layerId;

public:
  explicit ObserverRemoveLayer(state::LayerId layerId) : layerId(layerId) {}
  explicit ObserverRemoveLayer(const bmin::String& value)
      : layerId(state::layerIdFromString(bmin::toStringView(value))) {}
  explicit ObserverRemoveLayer(std::string_view value)
      : layerId(state::layerIdFromString(value)) {}

  void onClick(int mouseX, int mouseY, int button) override {
    LOG(INFO) << "ObserverRemoveLayer::onClick" << LOG_ENDL;
    auto stateManager = getStateManager();
    if (!stateManager || !layerId) {
      return;
    }
    stateManager->enqueueAction(state::makeAction<state::actions::UiRemoveLayer>(*layerId), 0);
  }
};
} // namespace ui
