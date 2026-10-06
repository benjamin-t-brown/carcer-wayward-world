#pragma once

#include "bmin/DynArray.h"
#include "bmin/String.h"
#include "bmin/Map.h"
#include "model/instances/MapInstance.h"
#include "model/instances/World.hpp"

namespace db {
class Database;
}

namespace model {
struct Player;
}

namespace in3 {

struct StringEvaluatorFuncs {
  bmin::Map<bmin::String, bmin::String>& storage;
  // Optional game context for inventory grants / world mutations (SpecialEventRunner).
  model::Player* player = nullptr;
  const db::Database* database = nullptr;
  model::ActiveMap* activeMap = nullptr;
  bmin::Map<bmin::String, model::MapInstance>* mapInstances = nullptr;

  bool questUpdated = false;
  bmin::DynArray<bmin::String> receivedItemNames;
  // Signed coins actually applied by MODIFY_COINS this eval (0 if none / clamped away).
  int modifiedCoins = 0;
  int modifiedExperience = 0;

  StringEvaluatorFuncs(bmin::Map<bmin::String, bmin::String>& storage);

  bmin::String GET(const bmin::String& a);
  void SET_BOOL(const bmin::String& a, const bmin::String& b);
  void SET_NUM(const bmin::String& a, const bmin::String& b);
  void MOD_NUM(const bmin::String& a, const bmin::String& b);
  void SET_STR(const bmin::String& a, const bmin::String& b);
  void SETUP_DISPOSITION(const bmin::String& characterName);
  void START_QUEST(const bmin::String& questName);
  void SET_QUEST_STEP_EQ(const bmin::String& questName, const bmin::String& stepId);
  void COMPLETE_QUEST_STEP(const bmin::String& questName, const bmin::String& stepId);
  void SHOW_QUEST_SUB_STEP(const bmin::String& questName, const bmin::String& stepId,
                           const bmin::String& subStepId);
  void HIDE_QUEST_SUB_STEP(const bmin::String& questName, const bmin::String& stepId,
                           const bmin::String& subStepId);
  void COMPLETE_QUEST_SUB_STEP(const bmin::String& questName, const bmin::String& stepId,
                               const bmin::String& subStepId);
  void COMPLETE_QUEST(const bmin::String& questName);
  void MODIFY_EXPERIENCE(const bmin::String& amount);
  void SPAWN_CH(const bmin::String& chName);
  void DESPAWN_CH(const bmin::String& chName);
  void CHANGE_TILE_AT(const bmin::String& x, const bmin::String& y, const bmin::String& tileName);
  // tileName is "tilesetName_tileId" (last '_' splits). Empty clears the cell.
  void CHANGE_TILE_AT_MARKER(const bmin::String& markerName, const bmin::String& tileName);
  void CHANGE_TILE_AT_MARKER(const bmin::String& markerName, const bmin::String& tileName,
                             const bmin::String& mapName);
  // Same as CHANGE_TILE_AT_MARKER, plus a changedTiles record that survives cross-grid loads.
  void CHANGE_TILE_AT_MARKER_PERMANENT(const bmin::String& markerName,
                                       const bmin::String& tileName);
  void CHANGE_TILE_AT_MARKER_PERMANENT(const bmin::String& markerName,
                                       const bmin::String& tileName,
                                       const bmin::String& mapName);
  void TELEPORT_TO(const bmin::String& x, const bmin::String& y, const bmin::String& mapName);
  void ADD_ITEM_AT(const bmin::String& x, const bmin::String& y, const bmin::String& itemName);
  void REMOVE_ITEM_AT(const bmin::String& x, const bmin::String& y, const bmin::String& itemName);
  void ADD_ITEM_TO_PLAYER(const bmin::String& itemName);
  void ADD_ITEM_TO_PLAYER(const bmin::String& itemName, const bmin::String& amount);
  void REMOVE_ITEM_FROM_PLAYER(const bmin::String& itemName);
  void MODIFY_COINS(const bmin::String& amount);
  void OPEN_SHOP(const bmin::String& shopName);
  void SET_PORT(const bmin::String& characterName);
};

class StringEvaluator {
public:
  bmin::String baseStringStr;
  StringEvaluatorFuncs funcs;
  bmin::String strResult;

  StringEvaluator(bmin::Map<bmin::String, bmin::String>& storage,
                  const bmin::String& baseStringStr);

  void assertFuncArgs(const bmin::String& funcName,
                      const bmin::DynArray<bmin::String>& funcArgs,
                      size_t expectedArgs);
  void evalStr(const bmin::String& str);
};

} // namespace in3
