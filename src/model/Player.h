#pragma once

#include "model/Character.h"
#include <string>

namespace model {
struct Player {
  std::string name;
  std::vector<model::CharacterPlayer> party;
  int currentPartyMemberIndex = 0;
  int gold = 0;
};
CharacterPlayer* playerFindPartyMemberById(Player& _player, const std::string& _id);
CharacterPlayer* playerFindPartyMemberByIndex(Player& _player, int _index);

} // namespace model
