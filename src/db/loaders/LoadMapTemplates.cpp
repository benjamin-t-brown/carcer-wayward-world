#include "LoadMapTemplates.h"
#include "bmin/StringInterop.h"
#include "lib/Json.h"
#include "sdl2w/AssetLoader.h"
#include <stdexcept>

namespace db {
namespace {

void loadTileOverrides(const Json& json, model::TileOverrides& overrides) {
  if (json.contains("isWalkableOverride")) {
    overrides.isWalkableOverride = json["isWalkableOverride"].get<bool>();
  }
  if (json.contains("isSeeThroughOverride")) {
    overrides.isSeeThroughOverride = json["isSeeThroughOverride"].get<bool>();
  }
  if (json.contains("isContainerOverride")) {
    overrides.isContainerOverride = json["isContainerOverride"].get<bool>();
  }
  if (json.contains("lightSourceOverride") && json["lightSourceOverride"].is_object()) {
    const auto& ls = json["lightSourceOverride"];
    model::TileLightSource light{};
    if (ls.contains("angle")) {
      light.angle = ls["angle"].get<int>();
    }
    if (ls.contains("intensity")) {
      light.intensity = ls["intensity"].get<int>();
    }
    if (ls.contains("radius")) {
      light.radius = ls["radius"].get<int>();
    }
    overrides.lightSourceOverride = light;
  }
}

void loadLightSource(const Json& json, model::TileLightSource& light) {
  if (json.contains("angle")) {
    light.angle = json["angle"].get<int>();
  }
  if (json.contains("intensity")) {
    light.intensity = json["intensity"].get<int>();
  }
  if (json.contains("radius")) {
    light.radius = json["radius"].get<int>();
  }
}

model::CarcerMapTemplate parseFlatMap(const Json& mapJson) {
  model::CarcerMapTemplate mapTemplate;

  if (!mapJson.contains("name") || !mapJson["name"].is_string()) {
    throw std::runtime_error("Map template missing name");
  }
  mapTemplate.name = mapJson["name"].get<bmin::String>();

  if (mapJson.contains("label") && mapJson["label"].is_string()) {
    mapTemplate.label = mapJson["label"].get<bmin::String>();
  }
  if (mapJson.contains("type") && mapJson["type"].is_string()) {
    mapTemplate.type = model::getMapTypeFromString(mapJson["type"].get<bmin::String>());
  }
  if (mapJson.contains("width")) {
    mapTemplate.width = mapJson["width"].get<int>();
  }
  if (mapJson.contains("height")) {
    mapTemplate.height = mapJson["height"].get<int>();
  }
  if (mapJson.contains("spriteWidth")) {
    mapTemplate.spriteWidth = mapJson["spriteWidth"].get<int>();
  }
  if (mapJson.contains("spriteHeight")) {
    mapTemplate.spriteHeight = mapJson["spriteHeight"].get<int>();
  }

  if (mapJson.contains("tilesets") && mapJson["tilesets"].is_array()) {
    for (const auto& tilesetJson : mapJson["tilesets"]) {
      mapTemplate.tilesets.pushBack(tilesetJson.get<bmin::String>());
    }
  }
  if (mapTemplate.tilesets.empty()) {
    mapTemplate.tilesets.pushBack("");
  }

  if (mapJson.contains("layers") && mapJson["layers"].is_array()) {
    for (const auto& layerJson : mapJson["layers"]) {
      mapTemplate.layers.pushBack(layerJson.get<int>());
    }
  }
  if (mapTemplate.layers.empty()) {
    mapTemplate.layers.pushBack(0);
  }

  if (mapJson.contains("tiles") && mapJson["tiles"].is_object()) {
    for (const auto& [layerKey, graphicsJson] : mapJson["tiles"].items()) {
      if (!graphicsJson.is_array()) {
        continue;
      }
      const int layer = bmin::parseInt(bmin::String(layerKey.c_str()));
      bmin::DynArray<int> graphics;
      graphics.reserve(graphicsJson.size());
      for (const auto& value : graphicsJson) {
        graphics.pushBack(value.get<int>());
      }
      if (layer >= 0) {
        const auto idx = static_cast<size_t>(layer);
        if (mapTemplate.tiles.size() <= idx) {
          mapTemplate.tiles.resize(idx + 1);
        }
        mapTemplate.tiles[idx] = std::move(graphics);
      }
    }
  }

  if (mapJson.contains("characters") && mapJson["characters"].is_array()) {
    for (const auto& entry : mapJson["characters"]) {
      model::MapCharacterPlacement placement;
      placement.l = entry.value("l", 0);
      placement.i = entry.value("i", 0);
      placement.name = entry.value("name", bmin::String());
      mapTemplate.characters.pushBack(std::move(placement));
    }
  }

  if (mapJson.contains("items") && mapJson["items"].is_array()) {
    for (const auto& entry : mapJson["items"]) {
      model::MapItemPlacement placement;
      placement.l = entry.value("l", 0);
      placement.i = entry.value("i", 0);
      placement.name = entry.value("name", bmin::String());
      placement.quantity = entry.value("quantity", 1);
      mapTemplate.items.pushBack(std::move(placement));
    }
  }

  if (mapJson.contains("markers") && mapJson["markers"].is_array()) {
    for (const auto& entry : mapJson["markers"]) {
      model::MapMarkerPlacement placement;
      placement.l = entry.value("l", 0);
      placement.i = entry.value("i", 0);
      placement.name = entry.value("name", bmin::String());
      mapTemplate.markers.pushBack(std::move(placement));
    }
  }

  if (mapJson.contains("eventTriggers") && mapJson["eventTriggers"].is_array()) {
    for (const auto& entry : mapJson["eventTriggers"]) {
      model::MapEventTriggerPlacement placement;
      placement.l = entry.value("l", 0);
      placement.i = entry.value("i", 0);
      placement.eventId = entry.value("eventId", bmin::String());
      placement.isNonCombatTrigger = entry.value("isNonCombatTrigger", true);
      placement.isLookTrigger = entry.value("isLookTrigger", false);
      mapTemplate.eventTriggers.pushBack(std::move(placement));
    }
  }

  if (mapJson.contains("travelTriggers") && mapJson["travelTriggers"].is_array()) {
    for (const auto& entry : mapJson["travelTriggers"]) {
      model::MapTravelTriggerPlacement placement;
      placement.l = entry.value("l", 0);
      placement.i = entry.value("i", 0);
      placement.destinationMapName = entry.value("destinationMapName", bmin::String());
      placement.destinationMarkerName = entry.value("destinationMarkerName", bmin::String());
      placement.destinationX = entry.value("destinationX", 0);
      placement.destinationY = entry.value("destinationY", 0);
      mapTemplate.travelTriggers.pushBack(std::move(placement));
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
      mapTemplate.tileOverrides.pushBack(std::move(placement));
    }
  }

  if (mapJson.contains("lightSources") && mapJson["lightSources"].is_array()) {
    for (const auto& entry : mapJson["lightSources"]) {
      model::MapLightSourcePlacement placement;
      placement.l = entry.value("l", 0);
      placement.i = entry.value("i", 0);
      loadLightSource(entry, placement);
      mapTemplate.lightSources.pushBack(std::move(placement));
    }
  }

  return mapTemplate;
}

} // namespace

void loadMapTemplates(const bmin::String& mapsFilePath,
                      bmin::Map<bmin::String, model::CarcerMapTemplate>& mapTemplates) {
  const bmin::String fileContent = sdl2w::loadFileAsString(bmin::toStringView(mapsFilePath));

  Json jsonData;
  try {
    jsonData = Json::parse(fileContent.cStr(), nullptr, true, true);
  } catch (const Json::parse_error& e) {
    throw std::runtime_error((bmin::String("Failed to parse maps JSON: ") + e.what()).cStr());
  }

  if (!jsonData.is_array()) {
    throw std::runtime_error("Maps JSON must be an array");
  }

  for (const auto& mapJson : jsonData) {
    model::CarcerMapTemplate mapTemplate = parseFlatMap(mapJson);
    if (mapTemplates.contains(mapTemplate.name)) {
      throw std::runtime_error((bmin::String("Duplicate map template name: ") + mapTemplate.name)
                                   .cStr());
    }
    mapTemplates[mapTemplate.name] = std::move(mapTemplate);
  }
}

} // namespace db
