#pragma once

#include "model/instances/CharacterPlayer.h"
#include "lib/Types.h"

namespace model {
struct Player {
  String name;
  bmin::DynArray<model::CharacterPlayer> party;
  int currentPartyMemberIndex = 0;
  int currentPartyMemberInventoryIndex = 0;
  int gold = 0;
  int food = 0;
};
CharacterPlayer* playerFindPartyMemberById(Player& _player, const String& _id);
CharacterPlayer* playerFindPartyMemberByIndex(Player& _player, int _index);

} // namespace model
