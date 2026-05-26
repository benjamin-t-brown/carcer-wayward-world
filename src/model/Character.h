#pragma once

#include "model/Items.h"
#include <optional>
#include <string>
#include <vector>

namespace db {
class Database;
}

namespace model {

enum class CharacterTemplateType {
  TOWNSPERSON,
  TOWNSPERSON_STATIC,
  ENEMY,
  ENEMY_STATIC,
};

struct CharacterTemplateTalk {
  std::string talkName;
  std::string portraitName;
};

struct CharacterTemplateBehavior {
  std::string behaviorName;
};

struct CharacterTemplateCombatStats {
  int str = 0;
  int mnd = 0;
  int con = 0;
  int agi = 0;
  int lck = 0;
};

struct CharacterTemplateCombat {
  CharacterTemplateCombatStats stats;
  int hp = 0;
  int mp = 0;
  std::string dropTable;
};

struct CharacterTemplateSound {
  std::string deathSoundName;
  std::string weaponSoundName;
};

struct CharacterTemplateStatus {
  std::string status;
};

struct CharacterTemplateVision {
  int radius;
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

// ----------------------------------------------------------------

struct CharacterTemplate {
  CharacterTemplateType type;
  std::string name;
  std::string label;
  std::string spritesheetName;
  std::string spriteOffset;
  CharacterTemplateTalk talk;
  CharacterTemplateBehavior behavior;
  CharacterTemplateCombat combat;
  CharacterTemplateSound sound;
  std::vector<CharacterTemplateStatus> statuses;
  CharacterTemplateVision vision;
};

struct Character {
  std::string id;
  std::string templateName;
};

std::string characterGetSprite(const Character& character, const db::Database* database);

struct CharacterPlayer : Character {
  std::vector<CharacterInventoryItem> inventory;
  CharacterPlayerEquipment equipment;
  CharacterTemplate params;

  CharacterPlayer();
};

std::string characterPlayerGetSprite(const CharacterPlayer& characterPlayer);

bool characterPlayerIsItemEquippedById(const CharacterPlayer& characterPlayer,
                                       const std::string& itemId);
std::optional<CharacterInventoryItem>
characterPlayerFindItemInInventoryByName(const CharacterPlayer& characterPlayer,
                                         const std::string& itemName);
void characterPlayerAddItemToInventory(CharacterPlayer& characterPlayer,
                                       const model::ItemTemplate& itemTemplate,
                                       int quantity = 1);
void characterPlayerRemoveItemFromInventoryByName(CharacterPlayer& characterPlayer,
                                                  const std::string& itemName,
                                                  int quantity = 1);
int characterGetWeightCarrying(const CharacterPlayer& characterPlayer,
                               const db::Database* database);
int characterGetWeightCapacity(const CharacterPlayer& characterPlayer);
std::vector<ItemInstance> characterGetNearbyItems(const CharacterPlayer& characterPlayer);

} // namespace model
