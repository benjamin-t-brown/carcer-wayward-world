#pragma once

#include "model/instances/CharacterPlayer.h"
namespace model {
struct Player {
  bmin::String name;
  bmin::DynArray<model::CharacterPlayer> party;
  int currentPartyMemberIndex = 0;
  int currentPartyMemberInventoryIndex = 0;
  int gold = 0;
  int food = 0;
};
CharacterPlayer* playerFindPartyMemberById(Player& _player, const bmin::String& _id);
CharacterPlayer* playerFindPartyMemberByIndex(Player& _player, int _index);
/** Index of party member with instanceId, or -1 if not found. */
int playerFindPartyMemberIndexById(const Player& _player, const bmin::String& _id);

} // namespace model
