#pragma once

#include "model/Character.h"
#include <string>

namespace model {
struct Player {
  std::string name;
  std::vector<model::CharacterPlayer> party;
  CharacterPlayer* findPartyMember(const std::string& id);
};
} // namespace model