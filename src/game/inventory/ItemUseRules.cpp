#include "game/inventory/ItemUseRules.h"
#include "db/Database.h"
#include "game/map/ActiveMapOrchestrator.h"
#include "model/Combat.h"

namespace game {

bool isItemUsableInContext(model::ItemUsability usability,
                           bool combatActive,
                           model::MapType mapType) {
  switch (usability) {
  case model::ItemUsability::NOT_USABLE:
    return false;
  case model::ItemUsability::USABLE_EVERYWHERE:
    return true;
  case model::ItemUsability::USABLE_COMBAT_ONLY:
    return combatActive;
  case model::ItemUsability::USABLE_TOWN_ONLY:
    return !combatActive && mapType == model::MapType::TOWN;
  case model::ItemUsability::USABLE_OUTSIDE_ONLY:
    return !combatActive && mapType == model::MapType::OUTDOOR;
  case model::ItemUsability::USABLE_TOWN_AND_COMBAT:
    return combatActive || mapType == model::MapType::TOWN;
  }
  return false;
}

bool isItemUsableNow(const model::ItemTemplate& itemTemplate,
                     bool combatActive,
                     model::MapType mapType) {
  if (!isItemUsableInContext(itemTemplate.itemUsability, combatActive, mapType)) {
    return false;
  }
  if (itemTemplate.useSpecialEvent.has_value() &&
      !itemTemplate.useSpecialEvent->empty()) {
    return true;
  }
  if (itemTemplate.useAbility.has_value() &&
      !itemTemplate.useAbility->abilityName.empty()) {
    return true;
  }
  return false;
}

model::MapType resolveActiveMapType(
    model::ActiveMap& activeMap,
    bmin::Map<bmin::String, model::MapInstance>& mapInstances,
    const db::Database* database,
    const model::Player& player) {
  if (database != nullptr && !activeMap.gridId.empty()) {
    try {
      ActiveMapOrchestrator orch(activeMap, mapInstances, database);
      for (const auto& character : activeMap.characters) {
        if (!model::isPartyMember(player, character.id)) {
          continue;
        }
        if (auto* map = orch.getMapInstanceAt(character.x, character.y)) {
          return map->mapType;
        }
      }
      if (auto* map = orch.getDefaultMapInstance()) {
        return map->mapType;
      }
    } catch (...) {
      // Fall through to mapInstances / TOWN default.
    }
  }

  for (auto it = mapInstances.begin(); it != mapInstances.end(); ++it) {
    return it->value.mapType;
  }
  return model::MapType::TOWN;
}

} // namespace game
