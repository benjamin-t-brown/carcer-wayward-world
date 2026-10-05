#include "StringEvaluator.h"
#include "EventRunnerHelpers.h"
#include "QuestProgress.h"
#include "bmin/StringInterop.h"
#include "db/Database.h"
#include "game/inventory/InventoryRules.h"
#include "model/instances/Player.h"
#include "sdl2w/Logger.h"
#include <cmath>
#include <stdexcept>

namespace in3 {

static int applyClampedDelta(bmin::Map<bmin::String, bmin::String>& storage,
                             const char* key,
                             const bmin::String& amount) {
  if (!bmin::isDouble(amount)) {
    throw std::runtime_error(("Invalid number value: " + amount).cStr());
  }
  const int delta = static_cast<int>(bmin::parseDouble(amount));
  auto current = getStorage(storage, bmin::String(key));
  int currentN = 0;
  if (current) {
    if (!bmin::isDouble(*current)) {
      throw std::runtime_error((bmin::String("Variable ") + key + " is not a number").cStr());
    }
    currentN = static_cast<int>(bmin::parseDouble(*current));
  }
  int next = currentN + delta;
  if (next < 0) {
    next = 0;
  }
  const int applied = next - currentN;
  setStorage(storage, bmin::String(key), bmin::toString(next));
  return applied;
}

static bmin::String formatNumber(double n) {
  double intPart;
  if (std::modf(n, &intPart) == 0.0) {
    return bmin::toString(static_cast<int>(n));
  }

  bmin::String result = bmin::toString(n);
  while (!result.empty() && result[result.size() - 1] == '0') {
    result.erase(result.size() - 1, 1);
  }
  if (!result.empty() && result[result.size() - 1] == '.') {
    result.erase(result.size() - 1, 1);
  }
  return result;
}

StringEvaluatorFuncs::StringEvaluatorFuncs(bmin::Map<bmin::String, bmin::String>& storage)
    : storage(storage) {}

bmin::String StringEvaluatorFuncs::GET(const bmin::String& a) {
  auto v = getStorage(storage, a);
  return v.value_or("");
}

void StringEvaluatorFuncs::SET_BOOL(const bmin::String& a, const bmin::String& b) {
  bool v = true;
  if (b == "true") {
    v = true;
  } else if (b == "false") {
    v = false;
  } else {
    v = true;
  }
  setStorage(storage, a, v ? "true" : "false");
}

void StringEvaluatorFuncs::SET_NUM(const bmin::String& a, const bmin::String& b) {
  if (!bmin::isDouble(b)) {
    throw std::runtime_error(("Invalid number value: " + b).cStr());
  }
  const double n = bmin::parseDouble(b);
  setStorage(storage, a, formatNumber(n));
}

void StringEvaluatorFuncs::MOD_NUM(const bmin::String& a, const bmin::String& b) {
  if (!bmin::isDouble(b)) {
    throw std::runtime_error(("Invalid number value: " + b).cStr());
  }
  const double n = bmin::parseDouble(b);
  auto current = getStorage(storage, a);
  double currentN = 0.0;
  if (current) {
    if (!bmin::isDouble(*current)) {
      throw std::runtime_error(("Variable " + a + " is not a number").cStr());
    }
    currentN = bmin::parseDouble(*current);
  }
  setStorage(storage, a, formatNumber(currentN + n));
}

void StringEvaluatorFuncs::SET_STR(const bmin::String& a, const bmin::String& b) {
  setStorage(storage, a, b);
}

void StringEvaluatorFuncs::SETUP_DISPOSITION(const bmin::String& characterName) {
  // noop
}

void StringEvaluatorFuncs::START_QUEST(const bmin::String& questName) {
  startQuest(storage, questName);
  questUpdated = true;
}

void StringEvaluatorFuncs::SET_QUEST_STEP_EQ(const bmin::String& questName,
                                             const bmin::String& stepId) {
  setQuestStepEq(storage, questName, stepId);
  questUpdated = true;
}

void StringEvaluatorFuncs::COMPLETE_QUEST_STEP(const bmin::String& questName,
                                               const bmin::String& stepId) {
  completeQuestStep(storage, questName, stepId);
  questUpdated = true;
}

void StringEvaluatorFuncs::SHOW_QUEST_SUB_STEP(const bmin::String& questName,
                                               const bmin::String& stepId,
                                               const bmin::String& subStepId) {
  showQuestSubStep(storage, questName, stepId, subStepId);
  questUpdated = true;
}

void StringEvaluatorFuncs::HIDE_QUEST_SUB_STEP(const bmin::String& questName,
                                               const bmin::String& stepId,
                                               const bmin::String& subStepId) {
  hideQuestSubStep(storage, questName, stepId, subStepId);
  questUpdated = true;
}

void StringEvaluatorFuncs::COMPLETE_QUEST_SUB_STEP(const bmin::String& questName,
                                                   const bmin::String& stepId,
                                                   const bmin::String& subStepId) {
  completeQuestSubStep(storage, questName, stepId, subStepId);
  questUpdated = true;
}

void StringEvaluatorFuncs::COMPLETE_QUEST(const bmin::String& questName) {
  const bool alreadyComplete = questIsComplete(storage, questName);
  completeQuest(storage, questName);
  questUpdated = true;
  if (alreadyComplete) {
    return;
  }
  const model::QuestTemplate* quest = findQuestTemplate(questName);
  if (!quest) {
    return;
  }
  for (size_t i = 0; i < quest->rewards.items.size(); ++i) {
    const auto& item = quest->rewards.items[i];
    if (!item.name.empty() && item.amount > 0) {
      ADD_ITEM_TO_PLAYER(item.name, bmin::toString(item.amount));
    }
  }
  if (quest->rewards.coins > 0) {
    MODIFY_COINS(bmin::toString(quest->rewards.coins));
  }
  if (quest->rewards.experience > 0) {
    MODIFY_EXPERIENCE(bmin::toString(quest->rewards.experience));
  }
}

void StringEvaluatorFuncs::SPAWN_CH(const bmin::String& chName) {
  // noop
}

void StringEvaluatorFuncs::DESPAWN_CH(const bmin::String& chName) {
  // noop
}

void StringEvaluatorFuncs::CHANGE_TILE_AT(const bmin::String& x, const bmin::String& y,
                                          const bmin::String& tileName) {
  // noop
}

void StringEvaluatorFuncs::TELEPORT_TO(const bmin::String& x, const bmin::String& y,
                                       const bmin::String& mapName) {
  // noop
}

void StringEvaluatorFuncs::ADD_ITEM_AT(const bmin::String& x, const bmin::String& y,
                                       const bmin::String& itemName) {
  // noop
}

void StringEvaluatorFuncs::REMOVE_ITEM_AT(const bmin::String& x, const bmin::String& y,
                                          const bmin::String& itemName) {
  // noop
}

void StringEvaluatorFuncs::ADD_ITEM_TO_PLAYER(const bmin::String& itemName) {
  ADD_ITEM_TO_PLAYER(itemName, "1");
}

void StringEvaluatorFuncs::ADD_ITEM_TO_PLAYER(const bmin::String& itemName,
                                              const bmin::String& amount) {
  if (itemName.empty()) {
    return;
  }
  const bmin::String qty = amount.empty() ? bmin::String("1") : amount;
  if (!bmin::isDouble(qty)) {
    throw std::runtime_error(("Invalid number value: " + qty).cStr());
  }
  const int n = static_cast<int>(bmin::parseDouble(qty));
  if (n <= 0) {
    return;
  }

  // No item DB (unit tests): keep legacy vars.items behavior.
  if (!database) {
    MOD_NUM(bmin::String("vars.items.") + itemName, bmin::toString(n));
    receivedItemNames.pushBack(itemName);
    return;
  }

  const auto* itemTemplate = database->findItemTemplate(bmin::toStringView(itemName));
  if (!itemTemplate) {
    LOG(ERROR) << "ADD_ITEM_TO_PLAYER: unknown item template '" << itemName << "'"
               << LOG_ENDL;
    return;
  }

  // Indestructable (quest/key) items stay in vars.items storage.
  if (itemTemplate->indestructable) {
    MOD_NUM(bmin::String("vars.items.") + itemName, bmin::toString(n));
    receivedItemNames.pushBack(itemName);
    return;
  }

  if (!player) {
    LOG(ERROR) << "ADD_ITEM_TO_PLAYER: no player context for inventory grant of '"
               << itemName << "'" << LOG_ENDL;
    return;
  }

  for (size_t i = 0; i < player->party.size(); ++i) {
    auto* member = model::playerFindPartyMemberByIndex(*player, static_cast<int>(i));
    if (!member) {
      continue;
    }
    if (!game::canAddItemToInventory(*member, *itemTemplate, n, *database)) {
      continue;
    }
    model::characterPlayerAddItemToInventory(*member, *itemTemplate, n);
    receivedItemNames.pushBack(itemName);
    return;
  }

  LOG(ERROR) << "ADD_ITEM_TO_PLAYER: no party member can carry '" << itemName << "' x"
             << n << LOG_ENDL;
}

void StringEvaluatorFuncs::MODIFY_COINS(const bmin::String& amount) {
  modifiedCoins += applyClampedDelta(storage, kPlayerCoinsStorageKey, amount);
}

void StringEvaluatorFuncs::MODIFY_EXPERIENCE(const bmin::String& amount) {
  modifiedExperience += applyClampedDelta(storage, kPlayerExperienceStorageKey, amount);
}

void StringEvaluatorFuncs::REMOVE_ITEM_FROM_PLAYER(const bmin::String& itemName) {
  // noop
}

void StringEvaluatorFuncs::OPEN_SHOP(const bmin::String& shopName) {
  // noop
}

void StringEvaluatorFuncs::SET_PORT(const bmin::String& characterName) {
  if (characterName.empty()) {
    storage.erase(bmin::String(kTalkPortStorageKey));
    return;
  }
  setStorage(storage, bmin::String(kTalkPortStorageKey), characterName);
}

StringEvaluator::StringEvaluator(bmin::Map<bmin::String, bmin::String>& storage,
                                 const bmin::String& baseStringStr)
    : baseStringStr(baseStringStr), funcs(storage) {}

void StringEvaluator::assertFuncArgs(const bmin::String& funcName,
                                     const bmin::DynArray<bmin::String>& funcArgs,
                                     size_t expectedArgs) {
  if (funcArgs.size() != expectedArgs) {
    throw std::runtime_error(
        ("Invalid number of arguments for function '" + funcName + "'. Expected " +
         bmin::toString(expectedArgs) + ", got " + bmin::toString(funcArgs.size()))
            .cStr());
  }
}

void StringEvaluator::evalStr(const bmin::String& str) {
  if (isFunctionCall(str)) {
    FunctionCall call = parseFunctionCall(str);
    if (call.funcName == "GET") {
      assertFuncArgs(call.funcName, call.args, 1);
      strResult = funcs.GET(call.args[0]);
    } else if (call.funcName == "SET_BOOL") {
      if (call.args.size() == 1) {
        funcs.SET_BOOL(call.args[0], "true");
      } else {
        assertFuncArgs(call.funcName, call.args, 2);
        funcs.SET_BOOL(call.args[0], call.args[1]);
      }
    } else if (call.funcName == "SET_NUM") {
      assertFuncArgs(call.funcName, call.args, 2);
      funcs.SET_NUM(call.args[0], call.args[1]);
    } else if (call.funcName == "MOD_NUM") {
      assertFuncArgs(call.funcName, call.args, 2);
      funcs.MOD_NUM(call.args[0], call.args[1]);
    } else if (call.funcName == "SET_STR") {
      assertFuncArgs(call.funcName, call.args, 2);
      funcs.SET_STR(call.args[0], call.args[1]);
    } else if (call.funcName == "SETUP_DISPOSITION") {
      assertFuncArgs(call.funcName, call.args, 1);
      funcs.SETUP_DISPOSITION(call.args[0]);
    } else if (call.funcName == "START_QUEST") {
      assertFuncArgs(call.funcName, call.args, 1);
      funcs.START_QUEST(call.args[0]);
    } else if (call.funcName == "SET_QUEST_STEP_EQ") {
      assertFuncArgs(call.funcName, call.args, 2);
      funcs.SET_QUEST_STEP_EQ(call.args[0], call.args[1]);
    } else if (call.funcName == "COMPLETE_QUEST_STEP") {
      assertFuncArgs(call.funcName, call.args, 2);
      funcs.COMPLETE_QUEST_STEP(call.args[0], call.args[1]);
    } else if (call.funcName == "SHOW_QUEST_SUB_STEP") {
      assertFuncArgs(call.funcName, call.args, 3);
      funcs.SHOW_QUEST_SUB_STEP(call.args[0], call.args[1], call.args[2]);
    } else if (call.funcName == "HIDE_QUEST_SUB_STEP") {
      assertFuncArgs(call.funcName, call.args, 3);
      funcs.HIDE_QUEST_SUB_STEP(call.args[0], call.args[1], call.args[2]);
    } else if (call.funcName == "COMPLETE_QUEST_SUB_STEP") {
      assertFuncArgs(call.funcName, call.args, 3);
      funcs.COMPLETE_QUEST_SUB_STEP(call.args[0], call.args[1], call.args[2]);
    } else if (call.funcName == "COMPLETE_QUEST") {
      assertFuncArgs(call.funcName, call.args, 1);
      funcs.COMPLETE_QUEST(call.args[0]);
    } else if (call.funcName == "SPAWN_CH") {
      assertFuncArgs(call.funcName, call.args, 1);
      funcs.SPAWN_CH(call.args[0]);
    } else if (call.funcName == "DESPAWN_CH") {
      assertFuncArgs(call.funcName, call.args, 1);
      funcs.DESPAWN_CH(call.args[0]);
    } else if (call.funcName == "CHANGE_TILE_AT") {
      assertFuncArgs(call.funcName, call.args, 3);
      funcs.CHANGE_TILE_AT(call.args[0], call.args[1], call.args[2]);
    } else if (call.funcName == "TELEPORT_TO") {
      assertFuncArgs(call.funcName, call.args, 3);
      funcs.TELEPORT_TO(call.args[0], call.args[1], call.args[2]);
    } else if (call.funcName == "ADD_ITEM_AT") {
      assertFuncArgs(call.funcName, call.args, 3);
      funcs.ADD_ITEM_AT(call.args[0], call.args[1], call.args[2]);
    } else if (call.funcName == "REMOVE_ITEM_AT") {
      assertFuncArgs(call.funcName, call.args, 3);
      funcs.REMOVE_ITEM_AT(call.args[0], call.args[1], call.args[2]);
    } else if (call.funcName == "ADD_ITEM_TO_PLAYER") {
      if (call.args.size() == 2) {
        funcs.ADD_ITEM_TO_PLAYER(call.args[0], call.args[1]);
      } else {
        assertFuncArgs(call.funcName, call.args, 1);
        funcs.ADD_ITEM_TO_PLAYER(call.args[0]);
      }
    } else if (call.funcName == "MODIFY_COINS") {
      assertFuncArgs(call.funcName, call.args, 1);
      funcs.MODIFY_COINS(call.args[0]);
    } else if (call.funcName == "MODIFY_EXPERIENCE") {
      assertFuncArgs(call.funcName, call.args, 1);
      funcs.MODIFY_EXPERIENCE(call.args[0]);
    } else if (call.funcName == "REMOVE_ITEM_FROM_PLAYER") {
      assertFuncArgs(call.funcName, call.args, 1);
      funcs.REMOVE_ITEM_FROM_PLAYER(call.args[0]);
    } else if (call.funcName == "OPEN_SHOP") {
      assertFuncArgs(call.funcName, call.args, 1);
      funcs.OPEN_SHOP(call.args[0]);
    } else if (call.funcName == "SET_PORT") {
      if (call.args.empty()) {
        funcs.SET_PORT("");
      } else {
        assertFuncArgs(call.funcName, call.args, 1);
        funcs.SET_PORT(call.args[0]);
      }
    } else {
      throw std::runtime_error(
          ("Function '" + call.funcName + "' not found: " + baseStringStr).cStr());
    }
  } else {
    throw std::runtime_error(("Invalid eval string: " + baseStringStr).cStr());
  }
}

} // namespace in3
