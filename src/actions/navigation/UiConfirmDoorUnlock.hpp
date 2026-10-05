#pragma once

#include "actions/general/PlaySound.hpp"
#include "game/map/ActiveMapOrchestrator.h"
#include "game/map/DoorLock.h"
#include "game/map/MapPersistence.h"
#include "game/map/MapVision.h"
#include "game/map/MapWalkability.h"
#include "game/map/TileTriggers.h"
#include "sdl2w/Logger.h"
#include "state/AbstractAction.hpp"
#include "state/State.hpp"

namespace state {

namespace actions {

// Confirms unlocking the closed locked door at active-map world (x, y).
class UiConfirmDoorUnlock : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::UiConfirmDoorUnlock; }
  int worldX = 0;
  int worldY = 0;

  void act() override {
    auto* database = getDatabase();
    if (!database) {
      LOG(ERROR) << "UiConfirmDoorUnlock::act: database is nullptr" << LOG_ENDL;
      return;
    }
    if (!state) {
      LOG(ERROR) << "UiConfirmDoorUnlock::act: state is nullptr" << LOG_ENDL;
      return;
    }

    auto& world = state->world;
    if (world.activeMap.gridId.empty()) {
      return;
    }
    if (state->player.party.empty()) {
      LOG(ERROR) << "UiConfirmDoorUnlock::act: party is empty" << LOG_ENDL;
      return;
    }

    game::ActiveMapOrchestrator orch(world.activeMap, state->mapInstances, database);
    auto* map = orch.getMapInstanceAt(worldX, worldY);
    const auto local = orch.activeMapCoordToInstanceCoord(worldX, worldY);
    if (!map || !local.valid) {
      LOG(WARN) << "UiConfirmDoorUnlock::act: map tile not found" << LOG_ENDL;
      removeLayerRequest(*state, LayerId::DoorUnlockConfirm);
      return;
    }
    map->tileLayerNumber = world.activeMap.mapLayer;

    auto* door = game::findClosedDoorAt(*map, local.x, local.y, *database);
    if (!door) {
      LOG(WARN) << "UiConfirmDoorUnlock::act: closed door not found" << LOG_ENDL;
      removeLayerRequest(*state, LayerId::DoorUnlockConfirm);
      return;
    }

    const auto opened = game::tryOpenClosedDoor(
        *door, state->player.party[0], state->specialEventStorage, *database);
    if (opened == game::ClosedDoorOpenResult::Blocked) {
      LOG(WARN) << "UiConfirmDoorUnlock::act: unlock blocked" << LOG_ENDL;
      removeLayerRequest(*state, LayerId::DoorUnlockConfirm);
      return;
    }

    game::persistClosedDoorOpen(map->persistentState,
                                map->tileLayerNumber,
                                door->x,
                                door->y,
                                door->tileId,
                                opened);
    if (opened == game::ClosedDoorOpenResult::OpenedKey) {
      PlaySound("unlock_door").execute(state);
    } else if (opened == game::ClosedDoorOpenResult::OpenedLockpick) {
      PlaySound("lockpick").execute(state);
    } else if (opened == game::ClosedDoorOpenResult::OpenedBash) {
      PlaySound("hit_punch1").execute(state);
    }

    if (auto* avatar = game::findPartyAvatarOnActiveMap(world.activeMap, state->player)) {
      game::updateActiveMapVisibilityFromPlayer(
          world, state->mapInstances, avatar->x, avatar->y, *database);
    }

    removeLayerRequest(*state, LayerId::DoorUnlockConfirm);
  }

public:
  UiConfirmDoorUnlock(int _worldX, int _worldY) : worldX(_worldX), worldY(_worldY) {}
};

} // namespace actions

} // namespace state
