#pragma once

#include "model/templates/RuneTypes.h"
#include "state/StateManager.h"
#include "state/actions/ui/UiAdjustEquippedRune.hpp"
#include "ui/UiElement.h"

namespace ui {

class ObserverAdjustEquippedRune : public ui::UiEventObserver,
                                   public state::StateManagerInterface {
  bmin::String characterPlayerId;
  model::RuneType runeType = model::RuneType::HEAT;
  int delta = 0;

public:
  ObserverAdjustEquippedRune(const bmin::String& _characterPlayerId,
                             model::RuneType _runeType,
                             int _delta)
      : characterPlayerId(_characterPlayerId), runeType(_runeType), delta(_delta) {}

  void onClick(int /*mouseX*/, int /*mouseY*/, int /*button*/) override {
    auto* stateManager = getStateManager();
    if (!stateManager || characterPlayerId.empty() || delta == 0) {
      return;
    }
    stateManager->enqueueAction(
        stateManager->getActionData(),
        new state::actions::UiAdjustEquippedRune(characterPlayerId, runeType, delta),
        0);
  }
};

} // namespace ui
