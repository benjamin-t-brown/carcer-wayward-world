#include "db/Database.h"
#include "model/TileTriggers.h"
#include "model/instances/CharacterPlayer.h"
#include "model/templates/Maps.h"
#include "sdl2w/Logger.h"
#include "state/DatabaseInterface.h"
#include "state/State.h"
#include "state/actions/world/WorldLoadMap.hpp"
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

    state::State state;
    state.player.party.pushBack(
        model::CharacterPlayer(database.getCharacterTemplate("testPartyMember1")));
    state.player.currentPartyMemberIndex = 0;
    const auto partyId = state.player.party[0].instanceId;

    // Start on alinea_outsideAlinea1 so travel must load a different map.
    {
      auto loadMap = state::actions::WorldLoadMap("alinea_outsideAlinea1");
      loadMap.execute(&state);
    }
    {
      auto spawn = state::actions::WorldSpawnPlayerAtXY(1, 1);
      spawn.execute(&state);
    }
    ok = assertTrue(model::findPartyAvatarOnMap(state.world.currentMap, state.player) !=
                        nullptr,
                    "avatar after XY spawn on start map") &&
         ok;

    // Marker path: AlineaTest MarkerHouseFloor2 is l=1, i=208, width=25 → (8, 8)
    {
      auto travel = model::TravelTrigger{};
      travel.destinationMapName = "AlineaTest";
      travel.destinationMarkerName = "MarkerHouseFloor2";
      travel.destinationX = 0;
      travel.destinationY = 0;
      state::actions::WorldTravel(travel).execute(&state);
    }
    ok = assertEqualStr(state.world.currentMap.templateName, "AlineaTest",
                        "map after marker travel") &&
         ok;
    ok = assertEqual(state.world.currentMap.tileLayerNumber, 1,
                     "tileLayerNumber after MarkerHouseFloor2") &&
         ok;
    {
      const auto* avatar =
          model::findPartyAvatarOnMap(state.world.currentMap, state.player);
      ok = assertTrue(avatar != nullptr, "avatar after marker travel") && ok;
      if (avatar) {
        ok = assertEqual(avatar->x, 8, "marker travel avatar.x") && ok;
        ok = assertEqual(avatar->y, 8, "marker travel avatar.y") && ok;
        ok = assertEqualStr(avatar->id, partyId, "marker travel avatar.id") && ok;
      }
    }

    // Missing marker falls back to XY and still creates avatar after map load.
    {
      auto travel = model::TravelTrigger{};
      travel.destinationMapName = "alinea_outsideAlinea1";
      travel.destinationMarkerName = "MissingMarkerDoesNotExist";
      travel.destinationX = 5;
      travel.destinationY = 6;
      state::actions::WorldTravel(travel).execute(&state);
    }
    ok = assertEqualStr(state.world.currentMap.templateName, "alinea_outsideAlinea1",
                        "map after XY fallback travel") &&
         ok;
    {
      const auto* avatar =
          model::findPartyAvatarOnMap(state.world.currentMap, state.player);
      ok = assertTrue(avatar != nullptr, "avatar after XY fallback travel") && ok;
      if (avatar) {
        ok = assertEqual(avatar->x, 5, "XY fallback avatar.x") && ok;
        ok = assertEqual(avatar->y, 6, "XY fallback avatar.y") && ok;
      }
    }

    // Empty marker name uses XY spawn-or-create after load.
    {
      auto travel = model::TravelTrigger{};
      travel.destinationMapName = "AlineaTest";
      travel.destinationMarkerName = "";
      travel.destinationX = 3;
      travel.destinationY = 4;
      state::actions::WorldTravel(travel).execute(&state);
    }
    ok = assertEqualStr(state.world.currentMap.templateName, "AlineaTest",
                        "map after empty-marker travel") &&
         ok;
    {
      const auto* avatar =
          model::findPartyAvatarOnMap(state.world.currentMap, state.player);
      ok = assertTrue(avatar != nullptr, "avatar after empty-marker travel") && ok;
      if (avatar) {
        ok = assertEqual(avatar->x, 3, "empty-marker travel avatar.x") && ok;
        ok = assertEqual(avatar->y, 4, "empty-marker travel avatar.y") && ok;
      }
    }

    // WorldSpawnPlayerAtXY alone creates after a fresh load (no prior avatar).
    {
      auto loadMap = state::actions::WorldLoadMap("AlineaTest");
      loadMap.execute(&state);
    }
    ok = assertTrue(model::findPartyAvatarOnMap(state.world.currentMap, state.player) ==
                        nullptr,
                    "avatar wiped after WorldLoadMap") &&
         ok;
    {
      auto spawn = state::actions::WorldSpawnPlayerAtXY(2, 3);
      spawn.execute(&state);
    }
    {
      const auto* avatar =
          model::findPartyAvatarOnMap(state.world.currentMap, state.player);
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
