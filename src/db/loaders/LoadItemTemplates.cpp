#include "LoadItemTemplates.h"
#include "db/spriteMappings.h"
#include "lib/sdl2w/AssetLoader.h"
#include "lib/json.hpp"

namespace db {

void loadItemTemplates(
    const std::string& itemsFilePath,
    std::unordered_map<std::string, model::ItemTemplate>& itemTemplates) {
  std::string fileContent = sdl2w::loadFileAsString(itemsFilePath);
  
  nlohmann::json jsonData;
  try {
    // Parse with comments enabled (ignore_comments = true)
    jsonData = nlohmann::json::parse(fileContent, nullptr, true, true);
  } catch (const nlohmann::json::parse_error& e) {
    throw std::runtime_error("Failed to parse JSON file " + itemsFilePath + ": " + e.what());
  }

  if (!jsonData.is_array()) {
    throw std::runtime_error("JSON file must contain an array of items");
  }

  for (const auto& itemJson : jsonData) {
    model::ItemTemplate itemTemplate;

    // Required fields
    if (!itemJson.contains("itemType") || !itemJson["itemType"].is_string()) {
      throw std::runtime_error("Item missing required field: itemType");
    }
    std::string itemTypeStr = itemJson["itemType"];
    auto itemType = model::getItemTypeFromString(itemTypeStr);
    if (itemType == model::ItemType::UNKNOWN) {
      throw std::runtime_error("Invalid item type: " + itemTypeStr);
    }
    itemTemplate.itemType = itemType;

    if (!itemJson.contains("name") || !itemJson["name"].is_string()) {
      throw std::runtime_error("Item missing required field: name");
    }
    itemTemplate.name = itemJson["name"];

    if (!itemJson.contains("label") || !itemJson["label"].is_string()) {
      throw std::runtime_error("Item missing required field: label");
    }
    itemTemplate.label = itemJson["label"];

    if (!itemJson.contains("icon") || !itemJson["icon"].is_string()) {
      throw std::runtime_error("Item missing required field: icon");
    }
    std::string iconName = itemJson["icon"];
    itemTemplate.iconSpriteName = db::getItemIconSpriteName(iconName);

    if (!itemJson.contains("description") || !itemJson["description"].is_string()) {
      throw std::runtime_error("Item missing required field: description");
    }
    itemTemplate.description = itemJson["description"];

    if (!itemJson.contains("weight") || !itemJson["weight"].is_number_integer()) {
      throw std::runtime_error("Item missing required field: weight");
    }
    itemTemplate.weight = itemJson["weight"];

    if (!itemJson.contains("value") || !itemJson["value"].is_number_integer()) {
      throw std::runtime_error("Item missing required field: value");
    }
    itemTemplate.value = itemJson["value"];

    // Check for duplicate names
    if (itemTemplates.find(itemTemplate.name) != itemTemplates.end()) {
      throw std::runtime_error("Item template already exists: " + itemTemplate.name);
    }

    itemTemplates[itemTemplate.name] = itemTemplate;
  }
}
} // namespace db

