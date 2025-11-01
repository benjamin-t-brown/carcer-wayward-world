#include "LoadItemTemplates.h"
#include "db/spriteMappings.h"
#include "lib/sdl2w/AssetLoader.h"

namespace db {

void loadItemTemplates(
    const std::string& itemsFilePath,
    std::unordered_map<std::string, model::ItemTemplate>& itemTemplates) {
  std::string fileContent = sdl2w::loadFileAsString(itemsFilePath);
  std::vector<std::string> lines;
  sdl2w::split(fileContent, "\n", lines);
  model::ItemTemplate itemTemplate;
  bool isInitialized = false;
  for (const std::string& line : lines) {
    if (sdl2w::trim(line).empty() || line[0] == '#' || line[0] == '/' || line[0] == ';') {
      continue;
    }
    std::vector<std::string> tokens;
    sdl2w::split(line, "|", tokens);

    if (tokens[0] == "ITEM") {
      if (isInitialized) {
        itemTemplates[itemTemplate.name] = itemTemplate;
      }
      isInitialized = true;
      itemTemplate = model::ItemTemplate(); // reset the item template
      if (tokens.size() < 7) {
        throw std::runtime_error("Invalid item template line: " + line);
      }
      auto itemType = model::getItemTypeFromString(tokens[1]);
      if (itemType == model::ItemType::UNKNOWN) {
        throw std::runtime_error("Invalid item type: " + tokens[1]);
      }
      itemTemplate.itemType = itemType;
      itemTemplate.name = sdl2w::trim(tokens[2]);
      itemTemplate.label = sdl2w::trim(tokens[3]);
      std::string iconName = sdl2w::trim(tokens[4]);
      itemTemplate.iconSpriteName = db::getItemIconSpriteName(iconName);
      itemTemplate.description = sdl2w::trim(tokens[5]);
      try {
        itemTemplate.weight = std::stoi(tokens[6]);
      } catch (const std::invalid_argument& e) {
        throw std::runtime_error("Invalid weight: " + tokens[6]);
      }
      try {
        itemTemplate.value = std::stoi(tokens[7]);
      } catch (const std::invalid_argument& e) {
        throw std::runtime_error("Invalid value: " + tokens[7]);
      }

      if (itemTemplates.find(itemTemplate.name) != itemTemplates.end()) {
        throw std::runtime_error("Item template already exists: " + itemTemplate.name);
      }
    }
  }

  if (isInitialized) {
    itemTemplates[itemTemplate.name] = itemTemplate;
  }
}
} // namespace db

