#include "LoadQuestTemplates.h"
#include "bmin/StringInterop.h"
#include "lib/Json.h"
#include "sdl2w/AssetLoader.h"
#include <stdexcept>

namespace db {
namespace {

bmin::String requireString(const Json& json, const char* field, const char* context) {
  if (!json.contains(field) || !json[field].is_string()) {
    throw std::runtime_error((bmin::String(context) + " missing required field: " + field).cStr());
  }
  return json[field].get<bmin::String>();
}

bmin::String optionalString(const Json& json, const char* field) {
  if (!json.contains(field) || !json[field].is_string()) {
    return {};
  }
  return json[field].get<bmin::String>();
}

int optionalNonNegativeInt(const Json& json, const char* field, const char* context) {
  if (!json.contains(field)) {
    return 0;
  }
  if (!json[field].is_number_integer()) {
    throw std::runtime_error(
        (bmin::String(context) + " field '" + field + "' must be an integer").cStr());
  }
  const int value = json[field].get<int>();
  if (value < 0) {
    throw std::runtime_error(
        (bmin::String(context) + " field '" + field + "' must be >= 0").cStr());
  }
  return value;
}

model::QuestRewards parseQuestRewards(const Json& questJson, const char* context) {
  model::QuestRewards rewards;
  if (!questJson.contains("rewards")) {
    return rewards;
  }
  if (!questJson["rewards"].is_object()) {
    throw std::runtime_error((bmin::String(context) + " rewards must be an object").cStr());
  }
  const Json& rewardsJson = questJson["rewards"];
  const bmin::String rewardsContext = bmin::String(context) + " rewards";
  rewards.coins = optionalNonNegativeInt(rewardsJson, "coins", rewardsContext.cStr());
  rewards.experience =
      optionalNonNegativeInt(rewardsJson, "experience", rewardsContext.cStr());
  if (rewardsJson.contains("items")) {
    if (!rewardsJson["items"].is_array()) {
      throw std::runtime_error((rewardsContext + " items must be an array").cStr());
    }
    for (const auto& itemJson : rewardsJson["items"]) {
      model::QuestRewardItem item;
      if (itemJson.is_string()) {
        item.name = itemJson.get<bmin::String>();
        item.amount = 1;
      } else if (itemJson.is_object()) {
        item.name = optionalString(itemJson, "name");
        if (item.name.empty()) {
          throw std::runtime_error((rewardsContext + " item is missing name").cStr());
        }
        item.amount = itemJson.contains("amount")
                          ? optionalNonNegativeInt(itemJson, "amount",
                                                   (rewardsContext + " item").cStr())
                          : 1;
      } else {
        throw std::runtime_error(
            (rewardsContext + " items must be strings or objects").cStr());
      }
      if (!item.name.empty() && item.amount > 0) {
        rewards.items.pushBack(item);
      }
    }
  }
  return rewards;
}

void assertUniqueIds(const bmin::DynArray<model::QuestStep>& steps, const char* context) {
  for (size_t i = 0; i < steps.size(); ++i) {
    if (steps[i].id.empty()) {
      throw std::runtime_error((bmin::String(context) + " has a step with empty id").cStr());
    }
    for (size_t j = i + 1; j < steps.size(); ++j) {
      if (steps[i].id == steps[j].id) {
        throw std::runtime_error(
            (bmin::String(context) + " has duplicate step id: " + steps[i].id.cStr()).cStr());
      }
    }
  }
}

model::QuestStep parseQuestStep(const Json& stepJson, bool allowSubSteps, const char* context) {
  model::QuestStep step;
  step.id = requireString(stepJson, "id", context);
  step.label = optionalString(stepJson, "label");
  step.description = optionalString(stepJson, "description");

  if (stepJson.contains("subSteps")) {
    if (!stepJson["subSteps"].is_array()) {
      throw std::runtime_error(
          (bmin::String(context) + " step '" + step.id.cStr() + "' subSteps must be an array")
              .cStr());
    }
    // Editor saves omit-or-empty `subSteps: []` on nested steps; empty is not nesting.
    if (stepJson["subSteps"].size() != 0) {
      if (!allowSubSteps) {
        throw std::runtime_error(
            (bmin::String(context) + " step '" + step.id.cStr() + "' cannot nest subSteps")
                .cStr());
      }
      const bmin::String subContext = bmin::String(context) + " step '" + step.id.cStr() + "'";
      for (const auto& subJson : stepJson["subSteps"]) {
        step.subSteps.pushBack(parseQuestStep(subJson, false, subContext.cStr()));
      }
      assertUniqueIds(step.subSteps, subContext.cStr());
    }
  }

  return step;
}

} // namespace

void loadQuestTemplates(const bmin::String& questsFilePath,
                        bmin::Map<bmin::String, model::QuestTemplate>& questTemplates) {
  const bmin::String fileContent = sdl2w::loadFileAsString(bmin::toStringView(questsFilePath));

  Json jsonData;
  try {
    jsonData = Json::parse(fileContent.cStr(), nullptr, true, true);
  } catch (const Json::parse_error& e) {
    throw std::runtime_error((bmin::String("Failed to parse JSON file ") + questsFilePath.cStr() +
                              ": " + e.what())
                                 .cStr());
  }

  if (!jsonData.is_array()) {
    throw std::runtime_error("JSON file must contain an array of quests");
  }

  for (const auto& questJson : jsonData) {
    model::QuestTemplate questTemplate;
    questTemplate.id = requireString(questJson, "id", "Quest");
    questTemplate.label = requireString(questJson, "label", "Quest");
    questTemplate.description = optionalString(questJson, "description");
    questTemplate.completedDescription = optionalString(questJson, "completedDescription");
    questTemplate.rewards = parseQuestRewards(
        questJson, (bmin::String("Quest '") + questTemplate.id.cStr() + "'").cStr());

    if (questJson.contains("steps")) {
      if (!questJson["steps"].is_array()) {
        throw std::runtime_error(
            (bmin::String("Quest '") + questTemplate.id.cStr() + "' steps must be an array").cStr());
      }
      const bmin::String context = bmin::String("Quest '") + questTemplate.id.cStr() + "'";
      for (const auto& stepJson : questJson["steps"]) {
        questTemplate.steps.pushBack(parseQuestStep(stepJson, true, context.cStr()));
      }
      assertUniqueIds(questTemplate.steps, context.cStr());
    }

    if (questTemplates.contains(questTemplate.id)) {
      throw std::runtime_error(
          (bmin::String("Quest template already exists: ") + questTemplate.id.cStr()).cStr());
    }
    questTemplates[questTemplate.id] = questTemplate;
  }
}

} // namespace db
