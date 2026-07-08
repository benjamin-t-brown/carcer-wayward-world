#include "LoadItemTemplates.h"
#include "db/loaders/LoadAbilityJson.h"
#include "lib/json.hpp"
#include "lib/sdl2w/AssetLoader.h"

namespace db {

void loadItemTemplates(
    const std::string& itemsFilePath,
    std::unordered_map<std::string, model::ItemTemplate>& itemTemplates) {
  std::string fileContent =
      std::string(sdl2w::loadFileAsString(itemsFilePath).sliceView());

  nlohmann::json jsonData;
  try {
    // Parse with comments enabled (ignore_comments = true)
    jsonData = nlohmann::json::parse(fileContent, nullptr, true, true);
  } catch (const nlohmann::json::parse_error& e) {
    throw std::runtime_error("Failed to parse JSON file " + itemsFilePath + ": " +
                             e.what());
  }

  if (!jsonData.is_array()) {
    throw std::runtime_error("JSON file must contain an array of items");
  }

  for (const auto& itemJson : jsonData) {
    model::ItemTemplate itemTemplate;

    // Required fields
    if (!itemJson.contains("itemType") || !itemJson["itemType"].is_string()) {
      throw std::runtime_error("Item missing required field: itemType");
    }
    std::string itemTypeStr = itemJson["itemType"];
    auto itemType = model::getItemTypeFromString(itemTypeStr);
    if (itemType == model::ItemType::UNKNOWN) {
      throw std::runtime_error("Invalid item type: " + itemTypeStr);
    }
    itemTemplate.itemType = itemType;

    if (!itemJson.contains("name") || !itemJson["name"].is_string()) {
      throw std::runtime_error("Item missing required field: name");
    }
    itemTemplate.name = itemJson["name"];

    if (!itemJson.contains("label") || !itemJson["label"].is_string()) {
      throw std::runtime_error("Item missing required field: label");
    }
    itemTemplate.label = itemJson["label"];

    if (!itemJson.contains("icon") || !itemJson["icon"].is_string()) {
      throw std::runtime_error("Item missing required field: icon");
    }
    itemTemplate.iconSpriteName = itemJson["icon"];

    if (!itemJson.contains("description") || !itemJson["description"].is_string()) {
      throw std::runtime_error("Item missing required field: description");
    }
    itemTemplate.description = itemJson["description"];

    if (!itemJson.contains("weight") || !itemJson["weight"].is_number_integer()) {
      throw std::runtime_error("Item missing required field: weight");
    }
    itemTemplate.weight = itemJson["weight"];

    if (!itemJson.contains("value") || !itemJson["value"].is_number_integer()) {
      throw std::runtime_error("Item missing required field: value");
    }
    itemTemplate.value = itemJson["value"];

    if (itemJson.contains("stackable") && itemJson["stackable"].is_boolean()) {
      itemTemplate.stackable = itemJson["stackable"];
    } else {
      itemTemplate.stackable = false;
    }

    if (itemJson.contains("indestructable") && itemJson["indestructable"].is_boolean()) {
      itemTemplate.indestructable = itemJson["indestructable"];
    } else {
      itemTemplate.indestructable = false;
    }

    if (itemJson.contains("statusEffects") && itemJson["statusEffects"].is_array()) {
      for (const auto& statusJson : itemJson["statusEffects"]) {
        if (statusJson.is_string()) {
          itemTemplate.statusEffectNames.push_back(statusJson.get<std::string>());
        } else if (statusJson.is_object() && statusJson.contains("name") &&
                   statusJson["name"].is_string()) {
          itemTemplate.statusEffectNames.push_back(statusJson["name"]);
        }
      }
    }

    if (itemJson.contains("itemUsability") && itemJson["itemUsability"].is_string()) {
      itemTemplate.itemUsability =
          model::getItemUsabilityFromString(itemJson["itemUsability"]);
    }

    if (itemJson.contains("useAbility") && itemJson["useAbility"].is_object()) {
      const auto& useAbilityJson = itemJson["useAbility"];
      if (useAbilityJson.contains("abilityName") &&
          useAbilityJson["abilityName"].is_string()) {
        model::ItemUseAbilityConfig useAbility;
        useAbility.abilityName = useAbilityJson["abilityName"];
        if (useAbilityJson.contains("dmgOverrides") &&
            useAbilityJson["dmgOverrides"].is_array()) {
          for (const auto& dmgJson : useAbilityJson["dmgOverrides"]) {
            if (dmgJson.is_object()) {
              useAbility.dmgOverrides.push_back(parseAbilityAttackDmg(dmgJson));
            }
          }
        }
        if (useAbilityJson.contains("restoreOverrides") &&
            useAbilityJson["restoreOverrides"].is_array()) {
          for (const auto& restoreJson : useAbilityJson["restoreOverrides"]) {
            if (restoreJson.is_object()) {
              useAbility.restoreOverrides.push_back(parseAbilityRestore(restoreJson));
            }
          }
        }
        itemTemplate.useAbility = useAbility;
      }
    }

    if (itemJson.contains("useSpecialEvent") && itemJson["useSpecialEvent"].is_string()) {
      const std::string useSpecialEvent = itemJson["useSpecialEvent"];
      if (!useSpecialEvent.empty()) {
        itemTemplate.useSpecialEvent = useSpecialEvent;
      }
    }

    if (itemJson.contains("weapon") && itemJson["weapon"].is_object()) {
      const auto& weaponJson = itemJson["weapon"];
      if (weaponJson.contains("abilityName") && weaponJson["abilityName"].is_string()) {
        model::ItemWeaponConfig weapon;
        weapon.abilityName = weaponJson["abilityName"];
        if (weaponJson.contains("dmgOverrides") &&
            weaponJson["dmgOverrides"].is_array()) {
          for (const auto& dmgJson : weaponJson["dmgOverrides"]) {
            if (dmgJson.is_object()) {
              weapon.dmgOverrides.push_back(parseAbilityAttackDmg(dmgJson));
            }
          }
        } else if (weaponJson.contains("dmg") && weaponJson["dmg"].is_object()) {
          int attackIndex = 0;
          if (weaponJson.contains("attackIndex") &&
              weaponJson["attackIndex"].is_number_integer()) {
            attackIndex = weaponJson["attackIndex"];
          }
          if (attackIndex > 0) {
            weapon.dmgOverrides.resize(static_cast<size_t>(attackIndex));
          }
          weapon.dmgOverrides.push_back(parseAbilityAttackDmg(weaponJson["dmg"]));
        }
        itemTemplate.weapon = weapon;
      }
    }

    // Check for duplicate names
    if (itemTemplates.find(itemTemplate.name) != itemTemplates.end()) {
      throw std::runtime_error("Item template already exists: " + itemTemplate.name);
    }

    itemTemplates[itemTemplate.name] = itemTemplate;
  }
}
} // namespace db
