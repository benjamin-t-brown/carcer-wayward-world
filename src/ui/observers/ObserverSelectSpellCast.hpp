#pragma once

#include "bmin/String.h"
#include "state/StateManager.h"
#include "actions/navigation/UiSelectSpellCast.hpp"
#include "ui/UiElement.h"

namespace ui {

class ObserverSelectSpellCast : public ui::UiEventObserver,
                                public state::StateManagerInterface {
  bmin::String spellId;
  bmin::String chId;

public:
  explicit ObserverSelectSpellCast(const bmin::String& _spellId,
                                   const bmin::String& _chId)
      : spellId(_spellId), chId(_chId) {}

  void onClick(int /*mouseX*/, int /*mouseY*/, int /*button*/) override {
    LOG(INFO) << "ObserverSelectSpellCast::onClick " << spellId << LOG_ENDL;
    auto stateManager = getStateManager();
    if (!stateManager || spellId.empty()) {
      return;
    }
    stateManager->enqueueAction(stateManager->getActionData(),
                                new state::actions::UiSelectSpellCast(spellId, chId),
                                0);
  }
};

} // namespace ui
