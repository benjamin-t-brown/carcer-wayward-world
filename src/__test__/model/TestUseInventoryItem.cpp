#include "actions/navigation/UiUseInventoryItem.hpp"
#include "db/Database.h"
#include "game/inventory/ItemUseRules.h"
#include "model/instances/CharacterPlayer.h"
#include "model/instances/MapInstance.h"
#include "model/instances/Player.h"
#include "model/templates/Abilities.hpp"
#include "model/templates/AbilityTypes.h"
#include "model/templates/Items.h"
#include "model/templates/Maps.h"
#include "sdl2w/Logger.h"
#include "state/DatabaseInterface.h"
#include "state/State.hpp"
#include "bmin/String.h"
#include <cassert>

#define TEST_NAME "TestUseInventoryItem"

namespace {

void addPotionHealAbility(db::Database& database) {
  model::AbilityTemplate ability;
  ability.name = "POTION_MINOR_HEAL";
  ability.type = model::AbilityType::ABILITY_SKILL;
  ability.targetSelect.targetType = model::TargetSelectType::TARGET_SELF;
  ability.apCost = 1;
  ability.depiction.startSound = "potion";
  ability.depiction.dmgSound = "spell_cure";
  model::AbilityRestore restore;
  restore.restoreWhich = model::CurrentStatEnum::CURRENT_STAT_HP;
  restore.restoreDice = {model::Dice::D0};
  restore.restoreBonus = 10;
  restore.restoreStat = model::StatsEnum::STAT_STR;
  restore.restoreStatMult = 0;
  ability.restores.pushBack(restore);
  database.addAbilityTemplate(ability);
}

void addHealingPotion(db::Database& database) {
  model::ItemTemplate itemTemplate;
  itemTemplate.name = "PotionHealing";
  itemTemplate.itemType = model::ItemType::POTION;
  itemTemplate.itemUsability = model::ItemUsability::USABLE_EVERYWHERE;
  itemTemplate.stackable = true;
  model::ItemUseAbilityConfig useAbility;
  useAbility.abilityName = "POTION_MINOR_HEAL";
  model::AbilityRestore restoreOverride;
  restoreOverride.restoreWhich = model::CurrentStatEnum::CURRENT_STAT_HP;
  restoreOverride.restoreDice = {model::Dice::D0};
  restoreOverride.restoreBonus = 7;
  restoreOverride.restoreStat = model::StatsEnum::STAT_CON;
  restoreOverride.restoreStatMult = 0;
  useAbility.restoreOverrides.pushBack(restoreOverride);
  itemTemplate.useAbility = useAbility;
  database.addItemTemplate(itemTemplate);
}

void addCombatOnlyPotion(db::Database& database) {
  model::ItemTemplate itemTemplate;
  itemTemplate.name = "CombatPotion";
  itemTemplate.itemType = model::ItemType::POTION;
  itemTemplate.itemUsability = model::ItemUsability::USABLE_COMBAT_ONLY;
  itemTemplate.stackable = true;
  model::ItemUseAbilityConfig useAbility;
  useAbility.abilityName = "POTION_MINOR_HEAL";
  itemTemplate.useAbility = useAbility;
  database.addItemTemplate(itemTemplate);
}

void addSpecialEventItem(db::Database& database) {
  model::ItemTemplate itemTemplate;
  itemTemplate.name = "AlineaCorrespondence1";
  itemTemplate.itemType = model::ItemType::UTILITY;
  itemTemplate.itemUsability = model::ItemUsability::USABLE_EVERYWHERE;
  itemTemplate.indestructable = true;
  itemTemplate.useSpecialEvent = bmin::String("item_useRealmOutpostCorrespondence");
  database.addItemTemplate(itemTemplate);
}

model::CharacterInventoryItem makeInventoryItem(const bmin::String& id,
                                                const bmin::String& itemName,
                                                int quantity = 1) {
  return {.itemName = itemName, .id = id, .quantity = quantity};
}

} // namespace

int main(int /*argc*/, char** /*argv*/) {
  LOG(INFO) << "Starting " << TEST_NAME << LOG_ENDL;

  // Usability matrix
  {
    assert(!game::isItemUsableInContext(model::ItemUsability::NOT_USABLE, false,
                                        model::MapType::TOWN));
    assert(game::isItemUsableInContext(model::ItemUsability::USABLE_EVERYWHERE, false,
                                       model::MapType::TOWN));
    assert(game::isItemUsableInContext(model::ItemUsability::USABLE_EVERYWHERE, true,
                                       model::MapType::OUTDOOR));
    assert(!game::isItemUsableInContext(model::ItemUsability::USABLE_COMBAT_ONLY, false,
                                        model::MapType::TOWN));
    assert(game::isItemUsableInContext(model::ItemUsability::USABLE_COMBAT_ONLY, true,
                                       model::MapType::TOWN));
    assert(game::isItemUsableInContext(model::ItemUsability::USABLE_TOWN_ONLY, false,
                                       model::MapType::TOWN));
    assert(!game::isItemUsableInContext(model::ItemUsability::USABLE_TOWN_ONLY, true,
                                        model::MapType::TOWN));
    assert(!game::isItemUsableInContext(model::ItemUsability::USABLE_TOWN_ONLY, false,
                                        model::MapType::OUTDOOR));
    assert(game::isItemUsableInContext(model::ItemUsability::USABLE_OUTSIDE_ONLY, false,
                                       model::MapType::OUTDOOR));
    assert(!game::isItemUsableInContext(model::ItemUsability::USABLE_OUTSIDE_ONLY, false,
                                        model::MapType::TOWN));
    assert(!game::isItemUsableInContext(model::ItemUsability::USABLE_OUTSIDE_ONLY, true,
                                        model::MapType::OUTDOOR));
    assert(game::isItemUsableInContext(model::ItemUsability::USABLE_TOWN_AND_COMBAT,
                                       false, model::MapType::TOWN));
    assert(game::isItemUsableInContext(model::ItemUsability::USABLE_TOWN_AND_COMBAT,
                                       true, model::MapType::OUTDOOR));
    assert(!game::isItemUsableInContext(model::ItemUsability::USABLE_TOWN_AND_COMBAT,
                                        false, model::MapType::OUTDOOR));
  }

  db::Database database;
  addPotionHealAbility(database);
  addHealingPotion(database);
  addCombatOnlyPotion(database);
  addSpecialEventItem(database);
  state::DatabaseInterface::setDatabase(&database);

  // Using PotionHealing heals and consumes one from the stack.
  {
    state::State state;
    state.world.combat.active = false;
    model::MapInstance map;
    map.templateName = "town_map";
    map.mapType = model::MapType::TOWN;
    state.mapInstances[map.templateName] = std::move(map);
    model::CharacterPlayer leader;
    leader.instanceId = "leader-id";
    leader.params.combat.hp = 20;
    leader.currentHp = 5;
    leader.inventory = {makeInventoryItem("potion-1", "PotionHealing", 2)};
    state.player.party = {leader};

    state::actions::UiUseInventoryItem("leader-id", "potion-1").execute(&state);

    assert(state.player.party[0].currentHp == 12);
    assert(state.player.party[0].inventory.size() == 1);
    assert(state.player.party[0].inventory[0].quantity == 1);
    assert(state.soundsToPlay.contains(bmin::String("potion")));
    assert(state.soundsToPlay.contains(bmin::String("spell_cure")));
  }

  // Combat-only item blocked in town (no combat).
  {
    state::State state;
    state.world.combat.active = false;
    model::MapInstance map;
    map.templateName = "town_map";
    map.mapType = model::MapType::TOWN;
    state.mapInstances[map.templateName] = std::move(map);
    model::CharacterPlayer leader;
    leader.instanceId = "leader-id";
    leader.params.combat.hp = 20;
    leader.currentHp = 5;
    leader.inventory = {makeInventoryItem("combat-potion-1", "CombatPotion", 1)};
    state.player.party = {leader};

    state::actions::UiUseInventoryItem("leader-id", "combat-potion-1").execute(&state);

    assert(state.player.party[0].currentHp == 5);
    assert(state.player.party[0].inventory.size() == 1);
    assert(!state.triggers.pendingSpecialEventId.has_value());
  }

  // Special-event use sets pendingSpecialEventId and keeps indestructable item.
  {
    state::State state;
    state.world.combat.active = false;
    model::CharacterPlayer leader;
    leader.instanceId = "leader-id";
    leader.inventory = {makeInventoryItem("scroll-1", "AlineaCorrespondence1", 1)};
    state.player.party = {leader};

    state::actions::UiUseInventoryItem("leader-id", "scroll-1").execute(&state);

    assert(state.triggers.pendingSpecialEventId.has_value());
    assert(*state.triggers.pendingSpecialEventId ==
           "item_useRealmOutpostCorrespondence");
    assert(state.player.party[0].inventory.size() == 1);
  }

  // Combat use spends AP on the map character.
  {
    state::State state;
    state.world.combat.active = true;
    model::CharacterPlayer leader;
    leader.instanceId = "leader-id";
    leader.params.combat.hp = 20;
    leader.currentHp = 5;
    leader.inventory = {makeInventoryItem("potion-1", "PotionHealing", 1)};
    state.player.party = {leader};
    model::CharacterInstance mapLeader;
    mapLeader.id = "leader-id";
    mapLeader.currentAp = 4;
    state.world.activeMap.characters = {mapLeader};

    state::actions::UiUseInventoryItem("leader-id", "potion-1").execute(&state);

    assert(state.player.party[0].currentHp == 12);
    assert(state.player.party[0].inventory.empty());
    assert(state.world.activeMap.characters[0].currentAp == 3);
  }

  state::DatabaseInterface::setDatabase(nullptr);

  LOG(INFO) << "Finished " << TEST_NAME << LOG_ENDL;
  return 0;
}
