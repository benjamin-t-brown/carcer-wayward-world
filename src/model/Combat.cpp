#include "model/Combat.h"
#include "model/TileTriggers.h"
#include "model/instances/CharacterInstance.h"
#include "model/instances/Player.h"
#include "model/instances/World.h"
#include "model/templates/CharacterTemplate.h"
#include "bmin/StringInterop.h"
#include "db/Database.h"

namespace model {

CharacterInstance* findCharacterOnMap(MapInstance& map, const bmin::String& id) {
  for (size_t i = 0; i < map.characters.size(); i++) {
    if (map.characters[i].id == id) {
      return &map.characters[i];
    }
  }
  return nullptr;
}

const CharacterInstance* findCharacterOnMap(const MapInstance& map, const bmin::String& id) {
  return findCharacterOnMap(const_cast<MapInstance&>(map), id);
}

bool isPartyMember(const Player& player, const bmin::String& characterId) {
  for (const auto& member : player.party) {
    if (member.instanceId == characterId) {
      return true;
    }
  }
  return false;
}

bool isCharacterEnemy(const CharacterInstance& character, const db::Database& database) {
  try {
    const auto& characterTemplate =
        database.getCharacterTemplate(bmin::toStringView(character.templateName));
    return characterTemplate.type == CharacterTemplateType::ENEMY ||
           characterTemplate.type == CharacterTemplateType::ENEMY_STATIC;
  } catch (...) {
    return false;
  }
}

bool isCharacterAlly(const Player& player,
                     const CharacterInstance& character,
                     const db::Database& database) {
  if (isPartyMember(player, character.id)) {
    return true;
  }
  return !isCharacterEnemy(character, database);
}

int getCharacterHp(const Player& player,
                   const CharacterInstance& character,
                   const db::Database& database) {
  if (isPartyMember(player, character.id)) {
    for (const auto& member : player.party) {
      if (member.instanceId == character.id) {
        return member.currentHp;
      }
    }
  }
  if (character.currentHp > 0) {
    return character.currentHp;
  }
  try {
    const auto& characterTemplate =
        database.getCharacterTemplate(bmin::toStringView(character.templateName));
    return characterTemplate.combat.hp;
  } catch (...) {
    return character.currentHp;
  }
}

void setCharacterHp(Player& player,
                    CharacterInstance& character,
                    int hp,
                    const db::Database& database) {
  if (isPartyMember(player, character.id)) {
    for (auto& member : player.party) {
      if (member.instanceId == character.id) {
        member.currentHp = hp;
        return;
      }
    }
  }
  (void)database;
  character.currentHp = hp;
}

bool isCharacterDefeated(const Player& player,
                         const CharacterInstance& character,
                         const db::Database& database) {
  return getCharacterHp(player, character, database) <= 0;
}

CharacterInstance* findCharacterAt(MapInstance& map,
                                   int x,
                                   int y,
                                   const bmin::String& excludeId) {
  for (size_t i = 0; i < map.characters.size(); i++) {
    auto& character = map.characters[i];
    if (character.x == x && character.y == y && character.id != excludeId) {
      return &character;
    }
  }
  return nullptr;
}

void resetAllCombatAp(World& world, int ap) {
  for (auto& character : world.currentMap.characters) {
    character.currentAp = ap;
  }
}

void addPartyMembersToCombatMap(World& world, Player& player, const db::Database& database) {
  auto& map = world.currentMap;
  auto* leader = findPartyAvatarOnMap(map, player);
  const auto spawnX = leader ? leader->x : 0;
  const auto spawnY = leader ? leader->y : 0;

  for (const auto& member : player.party) {
    if (findCharacterOnMap(map, member.instanceId) != nullptr) {
      continue;
    }

    auto instance = CharacterInstance{};
    instance.id = member.instanceId;
    instance.name = member.name.empty() ? member.params.name : member.name;
    instance.templateName =
        member.templateName.empty() ? member.params.name : member.templateName;
    instance.x = spawnX;
    instance.y = spawnY;
    instance.currentAp = COMBAT_STARTING_AP;
    instance.currentHp = member.currentHp;
    map.characters.pushBack(std::move(instance));
  }

  for (auto& character : map.characters) {
    if (character.currentHp <= 0 && isCharacterEnemy(character, database)) {
      try {
        const auto& characterTemplate =
            database.getCharacterTemplate(bmin::toStringView(character.templateName));
        character.currentHp = characterTemplate.combat.hp;
      } catch (...) {
      }
    }
  }
}

void removeExtraPartyMembersFromMap(World& world, const Player& player) {
  if (player.party.empty()) {
    return;
  }
  const auto& keepId = player.party[0].instanceId;
  auto& characters = world.currentMap.characters;
  for (size_t i = 0; i < characters.size();) {
    const auto& character = characters[i];
    if (isPartyMember(player, character.id) && character.id != keepId) {
      characters.erase(static_cast<size_t>(i));
      continue;
    }
    i++;
  }
}

Combat createCombatFromWorld(const World& world,
                             const Player& player,
                             const db::Database& database) {
  Combat combat;
  combat.active = true;
  combat.activeTurnIndex = 0;

  for (const auto& member : player.party) {
    if (findCharacterOnMap(world.currentMap, member.instanceId) != nullptr) {
      combat.turnOrderIds.pushBack(member.instanceId);
    }
  }

  for (const auto& character : world.currentMap.characters) {
    if (!isCharacterEnemy(character, database)) {
      continue;
    }
    auto alreadyListed = false;
    for (const auto& id : combat.turnOrderIds) {
      if (id == character.id) {
        alreadyListed = true;
        break;
      }
    }
    if (!alreadyListed) {
      combat.turnOrderIds.pushBack(character.id);
    }
  }

  return combat;
}

} // namespace model
