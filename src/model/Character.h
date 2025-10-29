#pragma once

#include "model/Items.h"
#include <optional>
#include <string>
#include <vector>

namespace model {
struct Character {
  std::string id;
  std::string name;
  std::string templateName;
};

struct CharacterPlayerEquipment {
  // these represent ids of items inside the character's inventory
  std::string weapon0Id;
  std::string weapon1Id;
  std::string ammoId;
  std::string hatId;
  std::string garbId;
  std::string glovesId;
  std::string pantsId;
  std::string shoesId;
  std::string necklaceId;
  std::string shieldId;
};

struct CharacterInventoryItem {
  std::string itemName;
  std::string id;
  int quantity;
};

struct CharacterPlayer : Character {
  std::vector<CharacterInventoryItem> inventory;
  CharacterPlayerEquipment equipment;

  CharacterPlayer();

  bool isItemEquippedById(const std::string& itemId) const;
  std::optional<CharacterInventoryItem>
  findItemInInventoryByName(const std::string& itemName) const;
  void addItemToInventory(const model::ItemTemplate& itemTemplate, int quantity = 1);
  void removeItemFromInventoryByName(const std::string& itemName, int quantity = 1);
};
} // namespace model