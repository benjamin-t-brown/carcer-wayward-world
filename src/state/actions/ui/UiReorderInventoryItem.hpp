#pragma once

#include "bmin/String.h"
#include "model/instances/CharacterPlayer.h"
#include "model/instances/Player.h"
#include "state/AbstractAction.h"
#include "state/State.h"

namespace state {

namespace actions {

class UiReorderInventoryItem : public AbstractAction {
  bmin::String characterPlayerId;
  int inventoryIndex = 0;
  int direction = 0;

  void act() override {
    auto& localState = *state;
    auto* characterPlayer =
        model::playerFindPartyMemberById(localState.player, characterPlayerId);
    if (!characterPlayer) {
      LOG(WARN) << "UiReorderInventoryItem::act: character not found " << characterPlayerId
                << LOG_ENDL;
      return;
    }
    if (!model::characterPlayerReorderInventoryItem(
            *characterPlayer, static_cast<size_t>(inventoryIndex), direction)) {
      LOG(WARN) << "UiReorderInventoryItem::act: reorder failed index="
                << inventoryIndex << " direction=" << direction << LOG_ENDL;
    }
  }

public:
  UiReorderInventoryItem(const bmin::String& _characterPlayerId,
                         int _inventoryIndex,
                         int _direction)
      : characterPlayerId(_characterPlayerId),
        inventoryIndex(_inventoryIndex),
        direction(_direction) {}
};

} // namespace actions

} // namespace state
