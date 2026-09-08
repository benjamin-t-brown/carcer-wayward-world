#include "game/combat/CombatParty.h"

#include "db/Database.h"
#include "game/map/CharacterConstruction.h"
#include "model/Combat.h"
#include "model/instances/CharacterInstance.hpp"
#include "model/instances/Player.h"
#include "model/instances/World.hpp"
#include <utility>

namespace game {

void addPartyMembersToCombatMap(model::World& world,
                                model::Player& player,
                                const db::Database& database) {
  auto& activeMap = world.activeMap;
  model::CharacterInstance* leader = nullptr;
  if (!player.party.empty()) {
    const auto& leaderId = player.party[0].instanceId;
    for (auto& character : activeMap.characters) {
      if (character.id == leaderId) {
        leader = &character;
        break;
      }
    }
  }
  const auto spawnX = leader ? leader->x : 0;
  const auto spawnY = leader ? leader->y : 0;

  for (const auto& member : player.party) {
    bool found = false;
    for (const auto& character : activeMap.characters) {
      if (character.id == member.instanceId) {
        found = true;
        break;
      }
    }
    if (found) {
      continue;
    }

    auto instance = model::CharacterInstance{};
    instance.id = member.instanceId;
    instance.name = member.name.empty() ? member.params.name : member.name;
    instance.templateName =
        member.templateName.empty() ? member.params.name : member.templateName;
    instance.x = spawnX;
    instance.y = spawnY;
    instance.spawnX = spawnX;
    instance.spawnY = spawnY;
    instance.currentAp = model::COMBAT_STARTING_AP;
    instance.currentHp = member.currentHp;
    applyCharacterTemplateFromDatabase(instance, database);
    activeMap.characters.pushBack(std::move(instance));
  }

  for (auto& character : activeMap.characters) {
    if (character.currentHp <= 0 && model::isCharacterEnemy(character)) {
      character.currentHp = character.maxHp;
    }
  }
}

} // namespace game
