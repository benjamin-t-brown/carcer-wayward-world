#pragma once

#include "state/StateManager.h"
#include "state/actions/ui/UiShowLayerPopupText.hpp"
#include "ui/UiElement.h"

namespace ui {

class ObserverShowLayerPopupText : public ui::UiEventObserver,
                                   public state::StateManagerInterface {
  sdl2w::Window* window;
  std::string title;
  std::string helpText;

public:
  ObserverShowLayerPopupText(sdl2w::Window* _window,
                             std::string _title,
                             std::string _helpText)
      : window(_window), title(std::move(_title)), helpText(std::move(_helpText)) {}

  void onClick(int mouseX, int mouseY, int button) override {
    LOG(INFO) << "ObserverShowLayerPopupText::onClick" << LOG_ENDL;
    auto stateManager = getStateManager();
    if (!stateManager || helpText.empty()) {
      return;
    }
    stateManager->enqueueAction(
        stateManager->getActionData(),
        new state::actions::UiShowLayerPopupText(window, title, helpText),
        0);
  }
};

} // namespace ui
