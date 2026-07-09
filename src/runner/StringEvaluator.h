#pragma once

#include "bmin/DynArray.h"
#include "bmin/String.h"
#include "lib/bmin/Map.h"

namespace runner {

struct StringEvaluatorFuncs {
  bmin::Map<bmin::String, bmin::String>& storage;

  StringEvaluatorFuncs(bmin::Map<bmin::String, bmin::String>& storage);

  bmin::String GET(const bmin::String& a);
  void SET_BOOL(const bmin::String& a, const bmin::String& b);
  void SET_NUM(const bmin::String& a, const bmin::String& b);
  void MOD_NUM(const bmin::String& a, const bmin::String& b);
  void SET_STR(const bmin::String& a, const bmin::String& b);
  void SETUP_DISPOSITION(const bmin::String& characterName);
  void START_QUEST(const bmin::String& questName);
  void COMPLETE_QUEST_STEP(const bmin::String& questName, const bmin::String& stepId);
  void COMPLETE_QUEST(const bmin::String& questName);
  void SPAWN_CH(const bmin::String& chName);
  void DESPAWN_CH(const bmin::String& chName);
  void CHANGE_TILE_AT(const bmin::String& x, const bmin::String& y, const bmin::String& tileName);
  void TELEPORT_TO(const bmin::String& x, const bmin::String& y, const bmin::String& mapName);
  void ADD_ITEM_AT(const bmin::String& x, const bmin::String& y, const bmin::String& itemName);
  void REMOVE_ITEM_AT(const bmin::String& x, const bmin::String& y, const bmin::String& itemName);
  void ADD_ITEM_TO_PLAYER(const bmin::String& itemName);
  void REMOVE_ITEM_FROM_PLAYER(const bmin::String& itemName);
  void OPEN_SHOP(const bmin::String& shopName);
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

} // namespace runner
