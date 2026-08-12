#pragma once

#include "bmin/String.h"
#include "state/StateManager.h"
#include "state/actions/ui/UiSetSpellReady.hpp"
#include "ui/UiElement.h"

namespace ui {

class ObserverSetSpellReady : public ui::UiEventObserver,
                              public state::StateManagerInterface {
  bmin::String characterPlayerId;
  bmin::String spellName;
  bool ready = true;

public:
  ObserverSetSpellReady(const bmin::String& _characterPlayerId,
                        const bmin::String& _spellName,
                        bool _ready)
      : characterPlayerId(_characterPlayerId), spellName(_spellName), ready(_ready) {}

  void onClick(int /*mouseX*/, int /*mouseY*/, int /*button*/) override {
    LOG(INFO) << "ObserverSetSpellReady::onClick spell=" << spellName
              << " ready=" << ready << LOG_ENDL;
    auto stateManager = getStateManager();
    if (!stateManager) {
      return;
    }
    stateManager->enqueueAction(
        stateManager->getActionData(),
        new state::actions::UiSetSpellReady(characterPlayerId, spellName, ready),
        0);
  }
};

} // namespace ui
