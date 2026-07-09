#include "LoadItemTemplates.h"
#include "db/loaders/LoadAbilityJson.h"
#include "lib/Json.h"
#include "lib/sdl2w/AssetLoader.h"

#include <stdexcept>

namespace db {

void loadItemTemplates(const String& itemsFilePath,
                       bmin::Map<String, model::ItemTemplate>& itemTemplates) {
  const String fileContent = sdl2w::loadFileAsString(bmin::toStringView(itemsFilePath));

  Json jsonData;
  try {
    jsonData = Json::parse(fileContent.cStr(), nullptr, true, true);
  } catch (const Json::parse_error& e) {
    throw std::runtime_error((String("Failed to parse JSON file ") + itemsFilePath.cStr() +
                              ": " + e.what())
                                 .cStr());
  }

  if (!jsonData.is_array()) {
    throw std::runtime_error("JSON file must contain an array of items");
  }

  for (const auto& itemJson : jsonData) {
    model::ItemTemplate itemTemplate;

    if (!itemJson.contains("itemType") || !itemJson["itemType"].is_string()) {
      throw std::runtime_error("Item missing required field: itemType");
    }
    const String itemTypeStr = itemJson["itemType"].get<String>();
    auto itemType = model::getItemTypeFromString(itemTypeStr);
    if (itemType == model::ItemType::UNKNOWN) {
      throw std::runtime_error((String("Invalid item type: ") + itemTypeStr).cStr());
    }
    itemTemplate.itemType = itemType;

    if (!itemJson.contains("name") || !itemJson["name"].is_string()) {
      throw std::runtime_error("Item missing required field: name");
    }
    itemTemplate.name = itemJson["name"].get<String>();

    if (!itemJson.contains("label") || !itemJson["label"].is_string()) {
      throw std::runtime_error("Item missing required field: label");
    }
    itemTemplate.label = itemJson["label"].get<String>();

    if (!itemJson.contains("icon") || !itemJson["icon"].is_string()) {
      throw std::runtime_error("Item missing required field: icon");
    }
    itemTemplate.iconSpriteName = itemJson["icon"].get<String>();

    if (!itemJson.contains("description") || !itemJson["description"].is_string()) {
      throw std::runtime_error("Item missing required field: description");
    }
    itemTemplate.description = itemJson["description"].get<String>();

    if (!itemJson.contains("weight") || !itemJson["weight"].is_number_integer()) {
      throw std::runtime_error("Item missing required field: weight");
    }
    itemTemplate.weight = itemJson["weight"].get<int>();

    if (!itemJson.contains("value") || !itemJson["value"].is_number_integer()) {
      throw std::runtime_error("Item missing required field: value");
    }
    itemTemplate.value = itemJson["value"].get<int>();

    if (itemJson.contains("stackable") && itemJson["stackable"].is_boolean()) {
      itemTemplate.stackable = itemJson["stackable"].get<bool>();
    } else {
      itemTemplate.stackable = false;
    }

    if (itemJson.contains("indestructable") && itemJson["indestructable"].is_boolean()) {
      itemTemplate.indestructable = itemJson["indestructable"].get<bool>();
    } else {
      itemTemplate.indestructable = false;
    }

    if (itemJson.contains("statusEffects") && itemJson["statusEffects"].is_array()) {
      for (const auto& statusJson : itemJson["statusEffects"]) {
        if (statusJson.is_string()) {
          itemTemplate.statusEffectNames.pushBack(statusJson.get<String>());
        } else if (statusJson.is_object() && statusJson.contains("name") &&
                   statusJson["name"].is_string()) {
          itemTemplate.statusEffectNames.pushBack(statusJson["name"].get<String>());
        }
      }
    }

    if (itemJson.contains("itemUsability") && itemJson["itemUsability"].is_string()) {
      itemTemplate.itemUsability =
          model::getItemUsabilityFromString(itemJson["itemUsability"].get<String>());
    }

    if (itemJson.contains("useAbility") && itemJson["useAbility"].is_object()) {
      const auto& useAbilityJson = itemJson["useAbility"];
      if (useAbilityJson.contains("abilityName") &&
          useAbilityJson["abilityName"].is_string()) {
        model::ItemUseAbilityConfig useAbility;
        useAbility.abilityName = useAbilityJson["abilityName"].get<String>();
        if (useAbilityJson.contains("dmgOverrides") &&
            useAbilityJson["dmgOverrides"].is_array()) {
          for (const auto& dmgJson : useAbilityJson["dmgOverrides"]) {
            if (dmgJson.is_object()) {
              useAbility.dmgOverrides.pushBack(parseAbilityAttackDmg(dmgJson));
            }
          }
        }
        if (useAbilityJson.contains("restoreOverrides") &&
            useAbilityJson["restoreOverrides"].is_array()) {
          for (const auto& restoreJson : useAbilityJson["restoreOverrides"]) {
            if (restoreJson.is_object()) {
              useAbility.restoreOverrides.pushBack(parseAbilityRestore(restoreJson));
            }
          }
        }
        itemTemplate.useAbility = useAbility;
      }
    }

    if (itemJson.contains("useSpecialEvent") && itemJson["useSpecialEvent"].is_string()) {
      const String useSpecialEvent = itemJson["useSpecialEvent"].get<String>();
      if (!useSpecialEvent.empty()) {
        itemTemplate.useSpecialEvent = useSpecialEvent;
      }
    }

    if (itemJson.contains("weapon") && itemJson["weapon"].is_object()) {
      const auto& weaponJson = itemJson["weapon"];
      if (weaponJson.contains("abilityName") && weaponJson["abilityName"].is_string()) {
        model::ItemWeaponConfig weapon;
        weapon.abilityName = weaponJson["abilityName"].get<String>();
        if (weaponJson.contains("dmgOverrides") &&
            weaponJson["dmgOverrides"].is_array()) {
          for (const auto& dmgJson : weaponJson["dmgOverrides"]) {
            if (dmgJson.is_object()) {
              weapon.dmgOverrides.pushBack(parseAbilityAttackDmg(dmgJson));
            }
          }
        } else if (weaponJson.contains("dmg") && weaponJson["dmg"].is_object()) {
          int attackIndex = 0;
          if (weaponJson.contains("attackIndex") &&
              weaponJson["attackIndex"].is_number_integer()) {
            attackIndex = weaponJson["attackIndex"].get<int>();
          }
          if (attackIndex > 0) {
            weapon.dmgOverrides.resize(static_cast<size_t>(attackIndex));
          }
          weapon.dmgOverrides.pushBack(parseAbilityAttackDmg(weaponJson["dmg"]));
        }
        itemTemplate.weapon = weapon;
      }
    }

    if (itemTemplates.contains(itemTemplate.name)) {
      throw std::runtime_error((String("Item template already exists: ") + itemTemplate.name)
                                   .cStr());
    }

    itemTemplates[itemTemplate.name] = itemTemplate;
  }
}
} // namespace db
