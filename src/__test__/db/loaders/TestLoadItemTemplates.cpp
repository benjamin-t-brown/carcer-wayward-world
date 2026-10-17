#include "db/loaders/LoadItemTemplates.h"
#include "sdl2w/Logger.h"
#include "model/templates/Items.h"
#include "bmin/String.h"
#include "bmin/Map.h"

int main(int argc, char** argv) {
  LOG(INFO) << "Starting TestLoadItemTemplates" << LOG_ENDL;

  bmin::Map<bmin::String, model::ItemTemplate> itemTemplates;

  try {
    db::loadItemTemplates("assets/db/items.json", itemTemplates);
    LOG(INFO) << "Successfully loaded " << itemTemplates.size() << " item templates"
              << LOG_ENDL;

    const auto daggerIt = itemTemplates.find(bmin::String("DaggerBronze"));
    if (daggerIt == itemTemplates.end()) {
      LOG(ERROR) << "Missing DaggerBronze item" << LOG_ENDL;
      return 1;
    }
    if (!daggerIt->value.weapon.has_value()) {
      LOG(ERROR) << "DaggerBronze should have weapon config" << LOG_ENDL;
      return 1;
    }
    if (daggerIt->value.weapon->abilityName != "MELEE_ATTACK_METAL_KNIFE") {
      LOG(ERROR) << "DaggerBronze weapon.abilityName should be MELEE_ATTACK_METAL_KNIFE"
                 << LOG_ENDL;
      return 1;
    }
    if (daggerIt->value.weapon->dmgOverrides.empty()) {
      LOG(ERROR) << "DaggerBronze should load dmgOverrides" << LOG_ENDL;
      return 1;
    }
    if (daggerIt->value.weapon->weaponVisual != model::WeaponVisual::Knife) {
      LOG(ERROR) << "DaggerBronze weapon.weaponVisual should be knife" << LOG_ENDL;
      return 1;
    }

    const auto swordIt = itemTemplates.find(bmin::String("SwordBronze"));
    if (swordIt == itemTemplates.end() || !swordIt->value.weapon.has_value() ||
        swordIt->value.weapon->weaponVisual != model::WeaponVisual::Sword) {
      LOG(ERROR) << "SwordBronze weapon.weaponVisual should be sword" << LOG_ENDL;
      return 1;
    }

    LOG(INFO) << "TestLoadItemTemplates completed successfully" << LOG_ENDL;
    return 0;
  } catch (const std::exception& e) {
    LOG(ERROR) << "Error loading item templates: " << e.what() << LOG_ENDL;
    return 1;
  }
}
