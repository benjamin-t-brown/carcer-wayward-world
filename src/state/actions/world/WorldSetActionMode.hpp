#pragma once

#include "model/instances/World.h"
#include "state/AbstractAction.h"
#include "state/State.h"

namespace state {

namespace actions {

class WorldSetActionMode : public AbstractAction {
  model::WorldActionMode mode = model::WorldActionMode::NONE;

  void act() override {
    if (!state) {
      return;
    }
    state->world.actionMode = mode;
  }

public:
  explicit WorldSetActionMode(model::WorldActionMode _mode) : mode(_mode) {}
};

} // namespace actions

} // namespace state
