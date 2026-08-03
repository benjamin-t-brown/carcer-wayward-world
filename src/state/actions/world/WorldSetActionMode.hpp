#pragma once

#include "game/map/TileTriggers.h"
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
    if (mode == model::WorldActionMode::NONE) {
      state->world.actionAimTile.reset();
      return;
    }

    const auto* avatar =
        game::findPartyAvatarOnActiveMap(state->world.activeMap, state->player);
    if (avatar) {
      state->world.actionAimTile = model::TileXY{avatar->x, avatar->y};
    } else {
      state->world.actionAimTile.reset();
    }
  }

public:
  explicit WorldSetActionMode(model::WorldActionMode _mode) : mode(_mode) {}
};

} // namespace actions

} // namespace state
