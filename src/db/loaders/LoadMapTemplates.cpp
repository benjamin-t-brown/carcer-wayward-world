#include "LoadMapTemplates.h"
#include "lib/json.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace db {
namespace {

void loadTileOverrides(const nlohmann::json& json, model::TileOverrides& overrides) {
  if (json.contains("isWalkableOverride")) {
    overrides.isWalkableOverride = json["isWalkableOverride"];
  }
  if (json.contains("isSeeThroughOverride")) {
    overrides.isSeeThroughOverride = json["isSeeThroughOverride"];
  }
  if (json.contains("isContainerOverride")) {
    overrides.isContainerOverride = json["isContainerOverride"];
  }
  if (json.contains("lightSourceOverride") && json["lightSourceOverride"].is_object()) {
    const auto& ls = json["lightSourceOverride"];
    model::TileLightSource light{};
    if (ls.contains("angle")) {
      light.angle = ls["angle"];
    }
    if (ls.contains("intensity")) {
      light.intensity = ls["intensity"];
    }
    if (ls.contains("radius")) {
      light.radius = ls["radius"];
    }
    overrides.lightSourceOverride = light;
  }
}

void loadLightSource(const nlohmann::json& json, model::TileLightSource& light) {
  if (json.contains("angle")) {
    light.angle = json["angle"];
  }
  if (json.contains("intensity")) {
    light.intensity = json["intensity"];
  }
  if (json.contains("radius")) {
    light.radius = json["radius"];
  }
}

model::CarcerMapTemplate parseFlatMap(const nlohmann::json& mapJson) {
  model::CarcerMapTemplate mapTemplate;

  if (!mapJson.contains("name") || !mapJson["name"].is_string()) {
    throw std::runtime_error("Map template missing name");
  }
  mapTemplate.name = mapJson["name"];

  if (mapJson.contains("label") && mapJson["label"].is_string()) {
    mapTemplate.label = mapJson["label"];
  }
  if (mapJson.contains("type") && mapJson["type"].is_string()) {
    mapTemplate.type = model::getMapTypeFromString(mapJson["type"]);
  }
  if (mapJson.contains("width")) {
    mapTemplate.width = mapJson["width"];
  }
  if (mapJson.contains("height")) {
    mapTemplate.height = mapJson["height"];
  }
  if (mapJson.contains("spriteWidth")) {
    mapTemplate.spriteWidth = mapJson["spriteWidth"];
  }
  if (mapJson.contains("spriteHeight")) {
    mapTemplate.spriteHeight = mapJson["spriteHeight"];
  }

  if (mapJson.contains("tilesets") && mapJson["tilesets"].is_array()) {
    for (const auto& tilesetJson : mapJson["tilesets"]) {
      mapTemplate.tilesets.push_back(tilesetJson.get<std::string>());
    }
  }
  if (mapTemplate.tilesets.empty()) {
    mapTemplate.tilesets.push_back("");
  }

  if (mapJson.contains("layers") && mapJson["layers"].is_array()) {
    for (const auto& layerJson : mapJson["layers"]) {
      mapTemplate.layers.push_back(layerJson.get<int>());
    }
  }
  if (mapTemplate.layers.empty()) {
    mapTemplate.layers.push_back(0);
  }

  if (mapJson.contains("tiles") && mapJson["tiles"].is_object()) {
    for (const auto& [layerKey, graphicsJson] : mapJson["tiles"].items()) {
      if (!graphicsJson.is_array()) {
        continue;
      }
      const int layer = std::stoi(layerKey);
      std::vector<int> graphics;
      graphics.reserve(graphicsJson.size());
      for (const auto& value : graphicsJson) {
        graphics.push_back(value.get<int>());
      }
      mapTemplate.tiles[layer] = std::move(graphics);
    }
  }

  if (mapJson.contains("characters") && mapJson["characters"].is_array()) {
    for (const auto& entry : mapJson["characters"]) {
      model::MapCharacterPlacement placement;
      placement.l = entry.value("l", 0);
      placement.i = entry.value("i", 0);
      placement.name = entry.value("name", "");
      mapTemplate.characters.push_back(std::move(placement));
    }
  }

  if (mapJson.contains("items") && mapJson["items"].is_array()) {
    for (const auto& entry : mapJson["items"]) {
      model::MapItemPlacement placement;
      placement.l = entry.value("l", 0);
      placement.i = entry.value("i", 0);
      placement.name = entry.value("name", "");
      placement.quantity = entry.value("quantity", 1);
      mapTemplate.items.push_back(std::move(placement));
    }
  }

  if (mapJson.contains("markers") && mapJson["markers"].is_array()) {
    for (const auto& entry : mapJson["markers"]) {
      model::MapMarkerPlacement placement;
      placement.l = entry.value("l", 0);
      placement.i = entry.value("i", 0);
      placement.name = entry.value("name", "");
      mapTemplate.markers.push_back(std::move(placement));
    }
  }

  if (mapJson.contains("eventTriggers") && mapJson["eventTriggers"].is_array()) {
    for (const auto& entry : mapJson["eventTriggers"]) {
      model::MapEventTriggerPlacement placement;
      placement.l = entry.value("l", 0);
      placement.i = entry.value("i", 0);
      placement.eventId = entry.value("eventId", "");
      placement.isNonCombatTrigger = entry.value("isNonCombatTrigger", true);
      placement.isLookTrigger = entry.value("isLookTrigger", false);
      mapTemplate.eventTriggers.push_back(std::move(placement));
    }
  }

  if (mapJson.contains("travelTriggers") && mapJson["travelTriggers"].is_array()) {
    for (const auto& entry : mapJson["travelTriggers"]) {
      model::MapTravelTriggerPlacement placement;
      placement.l = entry.value("l", 0);
      placement.i = entry.value("i", 0);
      placement.destinationMapName = entry.value("destinationMapName", "");
      placement.destinationMarkerName = entry.value("destinationMarkerName", "");
      placement.destinationX = entry.value("destinationX", 0);
      placement.destinationY = entry.value("destinationY", 0);
      mapTemplate.travelTriggers.push_back(std::move(placement));
    }
  }

  if (mapJson.contains("tileOverrides") && mapJson["tileOverrides"].is_array()) {
    for (const auto& entry : mapJson["tileOverrides"]) {
      model::MapTileOverridePlacement placement;
      placement.l = entry.value("l", 0);
      placement.i = entry.value("i", 0);
      if (entry.contains("overrides") && entry["overrides"].is_object()) {
        loadTileOverrides(entry["overrides"], placement.overrides);
      }
      mapTemplate.tileOverrides.push_back(std::move(placement));
    }
  }

  if (mapJson.contains("lightSources") && mapJson["lightSources"].is_array()) {
    for (const auto& entry : mapJson["lightSources"]) {
      model::MapLightSourcePlacement placement;
      placement.l = entry.value("l", 0);
      placement.i = entry.value("i", 0);
      loadLightSource(entry, placement);
      mapTemplate.lightSources.push_back(std::move(placement));
    }
  }

  return mapTemplate;
}

} // namespace

void loadMapTemplates(const std::string& mapsFilePath,
                      std::unordered_map<std::string, model::CarcerMapTemplate>& mapTemplates) {
  std::ifstream file(mapsFilePath);
  if (!file.is_open()) {
    throw std::runtime_error("Could not open maps file: " + mapsFilePath);
  }

  std::stringstream buffer;
  buffer << file.rdbuf();
  const std::string fileContent = buffer.str();

  nlohmann::json jsonData;
  try {
    jsonData = nlohmann::json::parse(fileContent, nullptr, true, true);
  } catch (const nlohmann::json::parse_error& e) {
    throw std::runtime_error("Failed to parse maps JSON: " + std::string(e.what()));
  }

  if (!jsonData.is_array()) {
    throw std::runtime_error("Maps JSON must be an array");
  }

  for (const auto& mapJson : jsonData) {
    model::CarcerMapTemplate mapTemplate = parseFlatMap(mapJson);
    if (mapTemplates.find(mapTemplate.name) != mapTemplates.end()) {
      throw std::runtime_error("Duplicate map template name: " + mapTemplate.name);
    }
    mapTemplates[mapTemplate.name] = std::move(mapTemplate);
  }
}

} // namespace db
