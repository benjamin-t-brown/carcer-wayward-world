#pragma once

#include "game/map/ActiveMapOrchestrator.h"
#include "model/instances/World.h"
#include "state/AbstractAction.h"
#include "state/State.h"

namespace state {

namespace actions {

class WorldLoadActiveMap : public AbstractAction {
  bmin::String gridId;

  void saveCurrentMapToPersistentState() {
    auto& localState = *state;

    auto previousGridId = localState.world.activeMap.gridId;
    if (previousGridId.empty()) {
      return;
    }
    game::ActiveMapOrchestrator previousActiveMap;
    previousActiveMap.fetchMapGrid(previousGridId);

    // save current map into persistentState
    for (const auto& ch : localState.world.activeMap.characters) {
      // copy ch data
      auto map = previousActiveMap.getMapInstanceAt(ch.x, ch.y);
      if (map) {
        map->persistentState.characters.pushBack(ch);
      } else {
        auto defaultMap = previousActiveMap.getDefaultMapInstance();
        if (defaultMap) {
          defaultMap->persistentState.characters.pushBack(ch);
        }
      }
    }
    for (const auto& item : localState.world.activeMap.items) {
      // copy item data
      auto map = previousActiveMap.getMapInstanceAt(item.x, item.y);
      if (map) {
        map->persistentState.items.pushBack(item);
      } else {
        auto defaultMap = previousActiveMap.getDefaultMapInstance();
        if (defaultMap) {
          defaultMap->persistentState.items.pushBack(item);
        }
      }
    }
  }

  void act() override {
    auto& localState = *state;

    saveCurrentMapToPersistentState();

    localState.world.activeMap = {};
    localState.world.activeMap.gridId = gridId;

    game::ActiveMapOrchestrator activeMap;
    activeMap.fetchMapGrid(gridId);
    auto& grid = activeMap.getMapGrid();
    for (int i = 0; i < grid.gridWidth; i++) {
      for (int j = 0; j < grid.gridHeight; j++) {
        const auto& mapName = grid.cells[i][j];
        if (mapName.empty()) {
          continue;
        }
        auto& map = localState.mapInstances[mapName];
        auto& persistentState = map.persistentState;
        for (const auto& character : persistentState.characters) {
          // copy ch data
          localState.world.activeMap.characters.pushBack(character);
        }
        for (const auto& item : persistentState.items) {
          // copy item data
          localState.world.activeMap.items.pushBack(item);
        }
        persistentState.characters.clear();
        persistentState.items.clear();
      }
    }
  }

public:
  WorldLoadActiveMap(const bmin::String& gridId) : gridId(gridId) {}
};

} // namespace actions

} // namespace state
