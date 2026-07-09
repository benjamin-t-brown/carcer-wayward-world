#pragma once

#include "lib/Types.h"
#include "lib/bmin/Map.h"

namespace runner {

struct StringEvaluatorFuncs {
  bmin::Map<String, String>& storage;

  StringEvaluatorFuncs(bmin::Map<String, String>& storage);

  String GET(const String& a);
  void SET_BOOL(const String& a, const String& b);
  void SET_NUM(const String& a, const String& b);
  void MOD_NUM(const String& a, const String& b);
  void SET_STR(const String& a, const String& b);
  void SETUP_DISPOSITION(const String& characterName);
  void START_QUEST(const String& questName);
  void COMPLETE_QUEST_STEP(const String& questName, const String& stepId);
  void COMPLETE_QUEST(const String& questName);
  void SPAWN_CH(const String& chName);
  void DESPAWN_CH(const String& chName);
  void CHANGE_TILE_AT(const String& x, const String& y, const String& tileName);
  void TELEPORT_TO(const String& x, const String& y, const String& mapName);
  void ADD_ITEM_AT(const String& x, const String& y, const String& itemName);
  void REMOVE_ITEM_AT(const String& x, const String& y, const String& itemName);
  void ADD_ITEM_TO_PLAYER(const String& itemName);
  void REMOVE_ITEM_FROM_PLAYER(const String& itemName);
  void OPEN_SHOP(const String& shopName);
};

class StringEvaluator {
public:
  String baseStringStr;
  StringEvaluatorFuncs funcs;
  String strResult;

  StringEvaluator(bmin::Map<String, String>& storage, const String& baseStringStr);

  void assertFuncArgs(const String& funcName, const DynArray<String>& funcArgs,
                      size_t expectedArgs);
  void evalStr(const String& str);
};

} // namespace runner
