#pragma once

#include <string>
#include <vector>

namespace types {
struct Character {
  std::string name;
  std::string templateName;
};

struct CharacterPlayerEquipment {
  std::string weapon0;
  std::string weapon1;
  std::string ammo;
  std::string hat;
  std::string garb;
  std::string gloves;
  std::string pants;
  std::string shoes;
  std::string necklace;
  std::string shield;
};

struct CharacterPlayer : Character {
  std::vector<std::string> inventory;
  CharacterPlayerEquipment equipment;
};
} // namespace types