#include "itemTemplates.h"
#include "db/spriteMappings.h"
#include "lib/sdl2w/AssetLoader.h"

namespace db {

void loadItemTemplates(
    const std::string& itemsFilePath,
    std::unordered_map<std::string, types::ItemTemplate>& itemTemplates) {
  std::string fileContent = sdl2w::loadFileAsString(itemsFilePath);
  std::vector<std::string> lines;
  sdl2w::split(fileContent, "\n", lines);
  for (const std::string& line : lines) {
    if (sdl2w::trim(line).empty() || line[0] == '#' || line[0] == '/' || line[0] == ';') {
      continue;
    }
    std::vector<std::string> tokens;
    sdl2w::split(line, ",", tokens);
    if (tokens.size() < 6) {
      throw std::runtime_error("Invalid item template line: " + line);
    }
    types::ItemTemplate itemTemplate;
    auto itemType = types::getItemTypeFromString(tokens[0]);
    if (itemType == types::ItemType::UNKNOWN) {
      throw std::runtime_error("Invalid item type: " + tokens[0]);
    }
    itemTemplate.itemType = itemType;
    itemTemplate.name = sdl2w::trim(tokens[1]);
    itemTemplate.label = sdl2w::trim(tokens[2]);
    std::string iconName = sdl2w::trim(tokens[3]);
    itemTemplate.iconSpriteName = db::getItemIconSpriteName(iconName);
    itemTemplate.description = sdl2w::trim(tokens[4]);
    try {
      itemTemplate.weight = std::stoi(tokens[5]);
    } catch (const std::invalid_argument& e) {
      throw std::runtime_error("Invalid weight: " + tokens[5]);
    }
    try {
      itemTemplate.value = std::stoi(tokens[6]);
    } catch (const std::invalid_argument& e) {
      throw std::runtime_error("Invalid value: " + tokens[6]);
    }

    if (itemTemplates.find(itemTemplate.name) != itemTemplates.end()) {
      throw std::runtime_error("Item template already exists: " + itemTemplate.name);
    }

    itemTemplates[itemTemplate.name] = itemTemplate;
  }
}
} // namespace db