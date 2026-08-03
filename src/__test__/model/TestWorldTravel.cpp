#include "db/Database.h"
#include "game/map/TileTriggers.h"
#include "model/instances/CharacterPlayer.h"
#include "model/templates/Maps.h"
#include "sdl2w/Logger.h"
#include "state/DatabaseInterface.h"
#include "state/State.h"
#include "state/StateManager.h"
#include "state/StateManagerInterface.h"
#include "state/actions/world/WorldLoadActiveMap.hpp"
#include "state/actions/world/WorldSpawnPlayerAtXY.hpp"
#include "state/actions/world/WorldTravel.hpp"
#include "bmin/String.h"

namespace {

bool assertEqual(int actual, int expected, const char* label) {
  if (actual != expected) {
    LOG(ERROR) << label << " expected " << expected << " but got " << actual << LOG_ENDL;
    return false;
  }
  return true;
}

bool assertEqualStr(const bmin::String& actual,
                    const bmin::String& expected,
                    const char* label) {
  if (actual != expected) {
    LOG(ERROR) << label << " expected " << expected << " but got " << actual << LOG_ENDL;
    return false;
  }
  return true;
}

bool assertTrue(bool cond, const char* label) {
  if (!cond) {
    LOG(ERROR) << label << " expected true" << LOG_ENDL;
    return false;
  }
  return true;
}

} // namespace

int main(int /*argc*/, char** /*argv*/) {
  LOG(INFO) << "Starting TestWorldTravel" << LOG_ENDL;

  bool ok = true;

  try {
    db::Database database;
    state::DatabaseInterface::setDatabase(&database);
    database.load();

    state::StateManager stateManager;
    state::StateManagerInterface::setStateManager(&stateManager);
    auto& state = stateManager.getState();
    state.player.party.pushBack(
        model::CharacterPlayer(database.getCharacterTemplate("testPartyMember1")));
    state.player.currentPartyMemberIndex = 0;
    const auto partyId = state.player.party[0].instanceId;

    {
      auto loadMap = state::actions::WorldLoadActiveMap("alinea_outsideAlinea1");
      loadMap.execute(&state);
    }
    {
      // World coords on OutsideAlinea for local (1,1) on alinea_outsideAlinea1 → (31,1)
      auto spawn = state::actions::WorldSpawnPlayerAtXY(31, 1);
      spawn.execute(&state);
    }
    ok = assertTrue(game::findPartyAvatarOnActiveMap(state.world.activeMap, state.player) !=
                        nullptr,
                    "avatar after XY spawn on start map") &&
         ok;

    {
      auto travel = model::TravelTrigger{};
      travel.destinationMapName = "AlineaTest";
      travel.destinationMarkerName = "MarkerHouseFloor2";
      travel.destinationX = 0;
      travel.destinationY = 0;
      state::actions::WorldTravel(travel).execute(&state);
    }
    ok = assertEqualStr(state.world.activeMap.gridId, "AlineaTest",
                        "grid after marker travel") &&
         ok;
    ok = assertEqual(state.world.activeMap.mapLayer, 1,
                     "mapLayer after MarkerHouseFloor2") &&
         ok;
    {
      const auto* avatar =
          game::findPartyAvatarOnActiveMap(state.world.activeMap, state.player);
      ok = assertTrue(avatar != nullptr, "avatar after marker travel") && ok;
      if (avatar) {
        ok = assertEqual(avatar->x, 8, "marker travel avatar.x") && ok;
        ok = assertEqual(avatar->y, 8, "marker travel avatar.y") && ok;
        ok = assertEqualStr(avatar->id, partyId, "marker travel avatar.id") && ok;
      }
    }

    {
      auto travel = model::TravelTrigger{};
      travel.destinationMapName = "alinea_outsideAlinea1";
      travel.destinationMarkerName = "MissingMarkerDoesNotExist";
      travel.destinationX = 5;
      travel.destinationY = 6;
      state::actions::WorldTravel(travel).execute(&state);
    }
    ok = assertEqualStr(state.world.activeMap.gridId, "OutsideAlinea",
                        "grid after XY fallback travel") &&
         ok;
    {
      const auto* avatar =
          game::findPartyAvatarOnActiveMap(state.world.activeMap, state.player);
      ok = assertTrue(avatar != nullptr, "avatar after XY fallback travel") && ok;
      if (avatar) {
        // local (5,6) on alinea_outsideAlinea1 → world (35,6)
        ok = assertEqual(avatar->x, 35, "XY fallback avatar.x") && ok;
        ok = assertEqual(avatar->y, 6, "XY fallback avatar.y") && ok;
      }
    }

    {
      auto travel = model::TravelTrigger{};
      travel.destinationMapName = "AlineaTest";
      travel.destinationMarkerName = "";
      travel.destinationX = 3;
      travel.destinationY = 4;
      state::actions::WorldTravel(travel).execute(&state);
    }
    ok = assertEqualStr(state.world.activeMap.gridId, "AlineaTest",
                        "grid after empty-marker travel") &&
         ok;
    {
      const auto* avatar =
          game::findPartyAvatarOnActiveMap(state.world.activeMap, state.player);
      ok = assertTrue(avatar != nullptr, "avatar after empty-marker travel") && ok;
      if (avatar) {
        ok = assertEqual(avatar->x, 3, "empty-marker travel avatar.x") && ok;
        ok = assertEqual(avatar->y, 4, "empty-marker travel avatar.y") && ok;
      }
    }

    // Same-grid travel must not unload/reload (NPC stays on activeMap).
    {
      auto npc = model::CharacterInstance{};
      npc.id = "same-grid-npc";
      npc.templateName = "same-grid-npc";
      npc.x = 1;
      npc.y = 1;
      state.world.activeMap.characters.pushBack(std::move(npc));
    }
    const auto charactersBeforeSameGrid = state.world.activeMap.characters.size();
    {
      auto travel = model::TravelTrigger{};
      travel.destinationMapName = "AlineaTest";
      travel.destinationMarkerName = "";
      travel.destinationX = 6;
      travel.destinationY = 7;
      travel.destinationLayer = 0;
      state::actions::WorldTravel(travel).execute(&state);
    }
    ok = assertEqualStr(state.world.activeMap.gridId, "AlineaTest",
                        "grid unchanged after same-grid travel") &&
         ok;
    ok = assertEqual(static_cast<int>(state.world.activeMap.characters.size()),
                     static_cast<int>(charactersBeforeSameGrid),
                     "same-grid travel keeps active characters") &&
         ok;
    {
      bool foundNpc = false;
      for (const auto& ch : state.world.activeMap.characters) {
        if (ch.id == "same-grid-npc") {
          foundNpc = true;
          break;
        }
      }
      ok = assertTrue(foundNpc, "same-grid travel keeps NPC on activeMap") && ok;
      const auto* avatar =
          game::findPartyAvatarOnActiveMap(state.world.activeMap, state.player);
      ok = assertTrue(avatar != nullptr, "avatar after same-grid travel") && ok;
      if (avatar) {
        ok = assertEqual(avatar->x, 6, "same-grid travel avatar.x") && ok;
        ok = assertEqual(avatar->y, 7, "same-grid travel avatar.y") && ok;
      }
    }

    {
      auto loadMap = state::actions::WorldLoadActiveMap("AlineaTest");
      loadMap.execute(&state);
    }
    ok = assertTrue(game::findPartyAvatarOnActiveMap(state.world.activeMap, state.player) ==
                        nullptr,
                    "avatar wiped after WorldLoadActiveMap") &&
         ok;
    {
      auto spawn = state::actions::WorldSpawnPlayerAtXY(2, 3);
      spawn.execute(&state);
    }
    {
      const auto* avatar =
          game::findPartyAvatarOnActiveMap(state.world.activeMap, state.player);
      ok = assertTrue(avatar != nullptr, "avatar created by WorldSpawnPlayerAtXY") && ok;
      if (avatar) {
        ok = assertEqual(avatar->x, 2, "XY create avatar.x") && ok;
        ok = assertEqual(avatar->y, 3, "XY create avatar.y") && ok;
      }
    }

    if (!ok) {
      LOG(ERROR) << "TestWorldTravel assertions failed" << LOG_ENDL;
      return 1;
    }

    LOG(INFO) << "TestWorldTravel completed successfully" << LOG_ENDL;
    return 0;
  } catch (const std::exception& e) {
    LOG(ERROR) << "Error: " << e.what() << LOG_ENDL;
    return 1;
  }
}
