#pragma once

#include "game/map/ActiveMapOrchestrator.h"
#include "game/map/MapPersistence.h"
#include "model/Combat.h"
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

    for (auto ch : localState.world.activeMap.characters) {
      if (model::isPartyMember(localState.player, ch.id)) {
        continue;
      }
      auto* map = previousActiveMap.getMapInstanceAt(ch.x, ch.y);
      if (!map) {
        map = previousActiveMap.getDefaultMapInstance();
      }
      if (!map) {
        continue;
      }
      const auto local = previousActiveMap.activeMapCoordToInstanceCoord(ch.x, ch.y);
      if (local.valid) {
        ch.x = local.x;
        ch.y = local.y;
      }
      map->persistentState.characters.pushBack(std::move(ch));
    }
    for (auto item : localState.world.activeMap.items) {
      auto* map = previousActiveMap.getMapInstanceAt(item.x, item.y);
      if (!map) {
        map = previousActiveMap.getDefaultMapInstance();
      }
      if (!map) {
        continue;
      }
      const auto local = previousActiveMap.activeMapCoordToInstanceCoord(item.x, item.y);
      if (local.valid) {
        item.x = local.x;
        item.y = local.y;
      }
      map->persistentState.items.pushBack(std::move(item));
    }
  }

  void act() override {
    auto& localState = *state;

    auto* database = getDatabase();
    if (!database) {
      return;
    }

    const auto resolvedGridId = game::resolveGridIdForMapOrGrid(*database, gridId);
    if (resolvedGridId.empty()) {
      return;
    }

    if (localState.mapInstances.empty()) {
      game::createMapInstances(localState, *database);
    }

    saveCurrentMapToPersistentState();

    localState.world.activeMap = {};
    localState.world.activeMap.gridId = resolvedGridId;
    localState.world.camera.camX = 0;
    localState.world.camera.camY = 0;
    localState.world.camera.cameraMode = model::CameraMode::Follow;
    localState.world.camera.cameraFollowCharacterId = bmin::String{};
    localState.world.actionMode = model::WorldActionMode::NONE;
    localState.world.actionAimTile.reset();

    game::ActiveMapOrchestrator activeMap;
    activeMap.fetchMapGrid(resolvedGridId);
    auto& grid = activeMap.getMapGrid();
    for (int y = 0; y < grid.gridHeight; y++) {
      for (int x = 0; x < grid.gridWidth; x++) {
        const auto& mapName = grid.cells[static_cast<size_t>(y)][static_cast<size_t>(x)];
        if (mapName.empty()) {
          continue;
        }
        auto it = localState.mapInstances.find(mapName);
        if (it == localState.mapInstances.end()) {
          continue;
        }
        auto& map = it->value;
        auto& persistentState = map.persistentState;

        // Drop defeated characters before hoisting.
        for (size_t ci = 0; ci < persistentState.characters.size();) {
          const auto& character = persistentState.characters[ci];
          auto remove = false;
          for (const auto& record : persistentState.defeatedCharacters) {
            if (character.templateName != record.templateName) {
              continue;
            }
            const auto spawnX = character.spawnX >= 0 ? character.spawnX : character.x;
            const auto spawnY = character.spawnY >= 0 ? character.spawnY : character.y;
            if (spawnX == record.x && spawnY == record.y) {
              remove = true;
              break;
            }
          }
          if (remove) {
            persistentState.characters.erase(ci);
          } else {
            ++ci;
          }
        }

        for (auto character : persistentState.characters) {
          const auto worldLoc =
              activeMap.instanceCoordToActiveMapCoord(mapName, character.x, character.y);
          if (worldLoc.valid) {
            character.x = worldLoc.x;
            character.y = worldLoc.y;
          }
          localState.world.activeMap.characters.pushBack(std::move(character));
        }
        for (auto item : persistentState.items) {
          const auto worldLoc =
              activeMap.instanceCoordToActiveMapCoord(mapName, item.x, item.y);
          if (worldLoc.valid) {
            item.x = worldLoc.x;
            item.y = worldLoc.y;
          }
          localState.world.activeMap.items.pushBack(std::move(item));
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
