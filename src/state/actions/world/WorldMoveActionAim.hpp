#pragma once

#include "game/map/ActiveMapOrchestrator.h"
#include "model/instances/World.h"
#include "state/AbstractAction.h"
#include "state/State.h"

namespace state {

namespace actions {

class WorldMoveActionAim : public AbstractAction {
  int dx = 0;
  int dy = 0;

  void act() override {
    if (!state) {
      return;
    }
    if (state->world.actionMode == model::WorldActionMode::NONE) {
      return;
    }
    if (!state->world.actionAimTile) {
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

    auto& aim = *state->world.actionAimTile;
    auto nextX = aim.x + dx;
    auto nextY = aim.y + dy;
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
    aim.x = nextX;
    aim.y = nextY;
  }

public:
  WorldMoveActionAim(int _dx, int _dy) : dx(_dx), dy(_dy) {}
};

} // namespace actions

} // namespace state
