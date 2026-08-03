#pragma once

#include "game/map/ActiveMapOrchestrator.h"
#include "model/instances/World.h"
#include "state/AbstractAction.h"
#include "state/State.h"

namespace state {

namespace actions {

class WorldSetActionAim : public AbstractAction {
  int x = 0;
  int y = 0;

  void act() override {
    if (!state) {
      return;
    }
    if (state->world.actionMode == model::WorldActionMode::NONE) {
      return;
    }
    if (state->world.activeMap.gridId.empty()) {
      return;
    }

    game::ActiveMapOrchestrator orch;
    orch.fetchMapGrid(state->world.activeMap.gridId);
    const auto total = orch.getTotalMapTilesSize();
    if (!total.valid || total.x <= 0 || total.y <= 0) {
      return;
    }

    auto nextX = x;
    auto nextY = y;
    if (nextX < 0) {
      nextX = 0;
    } else if (nextX >= total.x) {
      nextX = total.x - 1;
    }
    if (nextY < 0) {
      nextY = 0;
    } else if (nextY >= total.y) {
      nextY = total.y - 1;
    }

    if (state->world.actionAimTile && state->world.actionAimTile->x == nextX &&
        state->world.actionAimTile->y == nextY) {
      return;
    }
    state->world.actionAimTile = model::TileXY{nextX, nextY};
  }

public:
  WorldSetActionAim(int _x, int _y) : x(_x), y(_y) {}
};

} // namespace actions

} // namespace state
