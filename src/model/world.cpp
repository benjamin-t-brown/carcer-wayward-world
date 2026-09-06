module;
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <optional>
#include <utility>

module carcer.model;

import bmin.string_interop;

#include "macros.h"

// --- World.cppm ---

namespace model {

void resetAllCombatAp(World& world, int ap) {
  for (auto& character : world.activeMap.characters) {
    character.currentAp = ap;
  }
}

void addPartyMembersToCombatMap(World& world, Player& player, const db::Database& database) {
  auto& activeMap = world.activeMap;
  CharacterInstance* leader = nullptr;
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

    auto instance = CharacterInstance{};
    instance.id = member.instanceId;
    instance.name = member.name.empty() ? member.params.name : member.name;
    instance.templateName =
        member.templateName.empty() ? member.params.name : member.templateName;
    instance.x = spawnX;
    instance.y = spawnY;
    instance.spawnX = spawnX;
    instance.spawnY = spawnY;
    instance.currentAp = COMBAT_STARTING_AP;
    instance.currentHp = member.currentHp;
    tryApplyCharacterTemplateToInstance(instance, database);
    activeMap.characters.pushBack(std::move(instance));
  }

  for (auto& character : activeMap.characters) {
    if (character.currentHp <= 0 && isCharacterEnemy(character)) {
      character.currentHp = character.maxHp;
    }
  }
}

void removeExtraPartyMembersFromMap(World& world, const Player& player) {
  if (player.party.empty()) {
    return;
  }
  const auto& keepId = player.party[0].instanceId;
  auto& characters = world.activeMap.characters;
  for (size_t i = 0; i < characters.size();) {
    const auto& character = characters[i];
    if (isPartyMember(player, character.id) && character.id != keepId) {
      characters.erase(static_cast<size_t>(i));
      continue;
    }
    i++;
  }
}

Combat createCombatFromWorld(const World& world, const Player& player) {
  Combat combat;
  combat.active = true;
  combat.activeTurnIndex = 0;

  auto isInTurnOrder = [&](const bmin::String& id) {
    for (const auto& existingId : combat.turnOrderIds) {
      if (existingId == id) {
        return true;
      }
    }
    return false;
  };

  auto findOnActiveMap = [&](const bmin::String& id) {
    for (const auto& character : world.activeMap.characters) {
      if (character.id == id) {
        return true;
      }
    }
    return false;
  };

  for (const auto& member : player.party) {
    if (findOnActiveMap(member.instanceId) && !isInTurnOrder(member.instanceId)) {
      combat.turnOrderIds.pushBack(member.instanceId);
    }
  }

  for (const auto& character : world.activeMap.characters) {
    if (isInTurnOrder(character.id) || isCharacterEnemy(character)) {
      continue;
    }
    combat.turnOrderIds.pushBack(character.id);
  }

  for (const auto& character : world.activeMap.characters) {
    if (isInTurnOrder(character.id)) {
      continue;
    }
    combat.turnOrderIds.pushBack(character.id);
  }

  return combat;
}

bmin::String formatCharacterLogLabel(const ActiveMap& activeMap, const bmin::String& id) {
  const CharacterInstance* character = nullptr;
  for (const auto& ch : activeMap.characters) {
    if (ch.id == id) {
      character = &ch;
      break;
    }
  }
  if (character == nullptr || character->name.empty()) {
    return id;
  }
  bmin::StringStream ss;
  ss << character->name << " (" << id << ")";
  return bmin::String(ss.str().cStr());
}

} // namespace model
