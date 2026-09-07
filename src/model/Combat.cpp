#include "model/Combat.h"
#include "model/instances/CharacterInstance.h"
#include "model/instances/Player.h"
#include "model/instances/World.h"
#include "bmin/StringStream.h"

namespace model {

bool isPartyMember(const Player& player, const bmin::String& characterId) {
  for (const auto& member : player.party) {
    if (member.instanceId == characterId) {
      return true;
    }
  }
  return false;
}

bool isCharacterEnemy(const CharacterInstance& character) {
  return characterInstanceIsEnemy(character);
}

bool isCharacterAlly(const Player& player, const CharacterInstance& character) {
  if (isPartyMember(player, character.id)) {
    return true;
  }
  return !isCharacterEnemy(character);
}

int getCharacterHp(const Player& player, const CharacterInstance& character) {
  if (isPartyMember(player, character.id)) {
    for (const auto& member : player.party) {
      if (member.instanceId == character.id) {
        return member.currentHp;
      }
    }
  }
  if (character.hpInitialized) {
    return character.currentHp;
  }
  if (character.currentHp > 0) {
    return character.currentHp;
  }
  return character.maxHp;
}

void setCharacterHp(Player& player, CharacterInstance& character, int hp) {
  if (isPartyMember(player, character.id)) {
    for (auto& member : player.party) {
      if (member.instanceId == character.id) {
        member.currentHp = hp;
        return;
      }
    }
  }
  character.currentHp = hp;
  character.hpInitialized = true;
}

bool modifyPartyMemberHp(Player& player, const bmin::String& instanceId, int delta) {
  for (auto& member : player.party) {
    if (member.instanceId == instanceId) {
      member.currentHp += delta;
      return true;
    }
  }
  return false;
}

bool isCharacterDefeated(const Player& player, const CharacterInstance& character) {
  return getCharacterHp(player, character) <= 0;
}

void removeCharacterFromCombatTurnOrder(Combat& combat, const bmin::String& characterId) {
  for (size_t i = 0; i < combat.turnOrderIds.size();) {
    if (combat.turnOrderIds[i] != characterId) {
      ++i;
      continue;
    }
    combat.turnOrderIds.erase(i);
    if (combat.activeTurnIndex > static_cast<int>(i)) {
      combat.activeTurnIndex -= 1;
    } else if (!combat.turnOrderIds.empty() &&
               combat.activeTurnIndex >= static_cast<int>(combat.turnOrderIds.size())) {
      combat.activeTurnIndex = 0;
    }
  }
  if (combat.activeCharacterId == characterId) {
    combat.activeCharacterId = bmin::String{};
  }
}

void resetAllCombatAp(World& world, int ap) {
  for (auto& character : world.activeMap.characters) {
    character.currentAp = ap;
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
