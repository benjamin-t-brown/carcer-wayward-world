#pragma once

#include "bmin/DynArray.h"
#include "bmin/String.h"

namespace db {
class Database;
}

namespace model {

struct MapInstance;
struct Player;
struct World;
struct CharacterInstance;

inline constexpr int COMBAT_STARTING_AP = 4;
inline constexpr int COMBAT_MOVE_COST = 1;
inline constexpr int COMBAT_ATTACK_COST = 4;
inline constexpr int COMBAT_MELEE_DAMAGE = 10;
inline constexpr int COMBAT_HIT_CHANCE_PERCENT = 75;

enum class CombatActionType { MOVE, SHOOT, SPELL, WAIT };

struct CombatSpellTarget {
  bmin::String spellId;
  bmin::String targetCharacterId;
  int tileX = 0;
  int tileY = 0;
};

struct Combat {
  bool active = false;
  bmin::DynArray<bmin::String> turnOrderIds;
  int activeTurnIndex = 0;
  bmin::String activeCharacterId;
  bool isWaitingForAction = false;
};

CharacterInstance* findCharacterOnMap(MapInstance& map, const bmin::String& id);
const CharacterInstance* findCharacterOnMap(const MapInstance& map, const bmin::String& id);

bool isPartyMember(const Player& player, const bmin::String& characterId);
bool isCharacterAlly(const Player& player,
                     const CharacterInstance& character,
                     const db::Database& database);
bool isCharacterEnemy(const CharacterInstance& character, const db::Database& database);

int getCharacterHp(const Player& player,
                   const CharacterInstance& character,
                   const db::Database& database);
void setCharacterHp(Player& player,
                    CharacterInstance& character,
                    int hp,
                    const db::Database& database);
bool isCharacterDefeated(const Player& player,
                         const CharacterInstance& character,
                         const db::Database& database);

CharacterInstance* findCharacterAt(MapInstance& map,
                                   int x,
                                   int y,
                                   const bmin::String& excludeId = bmin::String{});

void resetAllCombatAp(World& world, int ap = COMBAT_STARTING_AP);
void addPartyMembersToCombatMap(World& world, Player& player, const db::Database& database);
void removeExtraPartyMembersFromMap(World& world, const Player& player);

Combat createCombatFromWorld(const World& world,
                             const Player& player,
                             const db::Database& database);

} // namespace model
