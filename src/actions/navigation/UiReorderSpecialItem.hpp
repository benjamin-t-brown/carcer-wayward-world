#pragma once

#include "game/inventory/SpecialEventItemsStorage.h"
#include "sdl2w/Logger.h"
#include "state/AbstractAction.hpp"
#include "state/State.hpp"

namespace state {

namespace actions {

class UiReorderSpecialItem : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::UiReorderSpecialItem; }
  int itemIndex = 0;
  int direction = 0;

  void act() override {
    if (!state) {
      return;
    }
    if (!game::reorderSpecialEventItemInStorage(state->specialEventStorage,
                                                static_cast<size_t>(itemIndex),
                                                direction)) {
      LOG(WARN) << "UiReorderSpecialItem::act: reorder failed index=" << itemIndex
                << " direction=" << direction << LOG_ENDL;
    }
  }

public:
  UiReorderSpecialItem(int _itemIndex, int _direction)
      : itemIndex(_itemIndex), direction(_direction) {}
};

} // namespace actions

} // namespace state
