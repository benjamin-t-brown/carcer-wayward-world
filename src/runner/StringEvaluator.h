#pragma once

#include "EventRunnerHelpers.h"
#include <string>
#include <unordered_map>

namespace runner {

struct StringEvaluatorFuncs {
  std::unordered_map<std::string, std::string>& storage;

  StringEvaluatorFuncs(std::unordered_map<std::string, std::string>& storage);

  std::string GET(const std::string& a);
  void SET_BOOL(const std::string& a, const std::string& b);
  void SET_NUM(const std::string& a, const std::string& b);
  void MOD_NUM(const std::string& a, const std::string& b);
  void SET_STR(const std::string& a, const std::string& b);
  void SETUP_DISPOSITION(const std::string& characterName);
  void START_QUEST(const std::string& questName);
  void ADVANCE_QUEST(const std::string& questName, const std::string& stepId);
  void COMPLETE_QUEST(const std::string& questName);
  void SPAWN_CH(const std::string& chName);
  void DESPAWN_CH(const std::string& chName);
  void CHANGE_TILE_AT(const std::string& x, const std::string& y,
                      const std::string& tileName);
  void TELEPORT_TO(const std::string& x, const std::string& y,
                   const std::string& mapName);
  void ADD_ITEM_AT(const std::string& x, const std::string& y,
                   const std::string& itemName);
  void REMOVE_ITEM_AT(const std::string& x, const std::string& y,
                      const std::string& itemName);
};

class StringEvaluator {
public:
  std::string baseStringStr;
  StringEvaluatorFuncs funcs;

  StringEvaluator(std::unordered_map<std::string, std::string>& storage,
                  const std::string& baseStringStr);

  void evalStr(const std::string& str);
};

} // namespace runner

