#pragma once

#include "bmin/String.h"
#include "state/StateManager.h"
#include "state/actions/ui/UiShowLayerSpellInfo.hpp"
#include "ui/UiElement.h"

namespace ui {

class ObserverShowLayerSpellInfo : public ui::UiEventObserver,
                                   public state::StateManagerInterface {
  sdl2w::Window* window;
  bmin::String spellName;

public:
  ObserverShowLayerSpellInfo(sdl2w::Window* _window, const bmin::String& _spellName)
      : window(_window), spellName(_spellName) {}

  void onClick(int /*mouseX*/, int /*mouseY*/, int /*button*/) override {
    LOG(INFO) << "ObserverShowLayerSpellInfo::onClick " << spellName << LOG_ENDL;
    auto stateManager = getStateManager();
    if (!stateManager || spellName.empty()) {
      return;
    }
    stateManager->enqueueAction(
        stateManager->getActionData(),
        new state::actions::UiShowLayerSpellInfo(window, spellName),
        0);
  }
};

} // namespace ui
