#include "spriteMappings.h"
#include <stdexcept>

namespace db {

std::string getItemIconSpriteName(const std::string& itemIcon,
                                  const std::string& spriteSheet) {
  int spriteOffset = -1;
  if (spriteSheet == "ui_item_icons") {
    if (itemIcon == "potionGreen") {
      spriteOffset = 0;
    } else if (itemIcon == "potionRed") {
      spriteOffset = 1;
    } else if (itemIcon == "potionBlue") {
      spriteOffset = 2;
    } else if (itemIcon == "potionPink") {
      spriteOffset = 3;
    } else if (itemIcon == "scrollRed") {
      spriteOffset = 4;
    } else if (itemIcon == "scrollBlue") {
      spriteOffset = 5;
    } else if (itemIcon == "scrollYellow") {
      spriteOffset = 6;
    } else if (itemIcon == "scrollPink") {
      spriteOffset = 7;
    } else if (itemIcon == "dagger") {
      spriteOffset = 8;
    } else if (itemIcon == "sword") {
      spriteOffset = 9;
    } else if (itemIcon == "swordMagic") {
      spriteOffset = 10;
    } else if (itemIcon == "spear") {
      spriteOffset = 11;
    } else if (itemIcon == "bow") {
      spriteOffset = 12;
    } else if (itemIcon == "axe") {
      spriteOffset = 13;
    } else if (itemIcon == "arrow") {
      spriteOffset = 14;
    } else if (itemIcon == "arrowMagic") {
      spriteOffset = 15;
    } else if (itemIcon == "cloak") {
      spriteOffset = 16;
    } else if (itemIcon == "shirtBlue") {
      spriteOffset = 17;
    } else if (itemIcon == "pants") {
      spriteOffset = 18;
    } else if (itemIcon == "armorChain") {
      spriteOffset = 19;
    } else if (itemIcon == "armorLeather") {
      spriteOffset = 20;
    } else if (itemIcon == "armorPlate") {
      spriteOffset = 21;
    } else if (itemIcon == "armorMail") {
      spriteOffset = 22;
    } else if (itemIcon == "bracer") {
      spriteOffset = 23;
    } else if (itemIcon == "bracerLarge") {
      spriteOffset = 24;
    } else if (itemIcon == "bucklerWood") {
      spriteOffset = 25;
    } else if (itemIcon == "bucklerMetal") {
      spriteOffset = 26;
    } else if (itemIcon == "shieldMetal") {
      spriteOffset = 27;
    } else if (itemIcon == "gloves") {
      spriteOffset = 28;
    } else if (itemIcon == "gauntlets") {
      spriteOffset = 29;
    } else if (itemIcon == "bootsLeather") {
      spriteOffset = 30;
    } else if (itemIcon == "bootsMetal") {
      spriteOffset = 31;
    } else if (itemIcon == "beer") {
      spriteOffset = 32;
    } else if (itemIcon == "bookGreen") {
      spriteOffset = 33;
    } else if (itemIcon == "bookRed") {
      spriteOffset = 34;
    } else if (itemIcon == "herbs") {
      spriteOffset = 35;
    } else if (itemIcon == "hatLeather") {
      spriteOffset = 36;
    } else if (itemIcon == "necklace") {
      spriteOffset = 37;
    }
  }

  if (spriteOffset == -1) {
    throw std::runtime_error("Sprite offset not found for item icon: " + itemIcon +
                             " in sprite sheet: " + spriteSheet);
  }

  return spriteSheet + "_" + std::to_string(spriteOffset);
}

} // namespace db