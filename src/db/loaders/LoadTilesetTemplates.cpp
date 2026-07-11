#include "LoadTilesetTemplates.h"
#include "bmin/StringInterop.h"
#include "lib/Json.h"
#include "sdl2w/AssetLoader.h"
#include <stdexcept>

namespace db {
namespace {

model::TileStepSound parseStepSound(const Json& value) {
  auto asInt = int{0};
  if (value.is_number_integer()) {
    asInt = value.get<int>();
  } else if (value.is_string()) {
    asInt = bmin::parseInt(value.get<bmin::String>());
  } else {
    return model::TILE_STEP_SOUND_FLOOR;
  }

  switch (asInt) {
  case 1:
    return model::TILE_STEP_SOUND_GRASS;
  case 2:
    return model::TILE_STEP_SOUND_DIRT;
  case 3:
    return model::TILE_STEP_SOUND_GRAVEL;
  case 0:
  default:
    return model::TILE_STEP_SOUND_FLOOR;
  }
}

model::TileMetadata parseTileMetadata(const Json& tileJson) {
  auto meta = model::TileMetadata{};
  if (tileJson.contains("id")) {
    meta.id = tileJson["id"].get<int>();
  }
  if (tileJson.contains("description") && tileJson["description"].is_string()) {
    meta.description = tileJson["description"].get<bmin::String>();
  }
  if (tileJson.contains("stepSound")) {
    meta.stepSound = parseStepSound(tileJson["stepSound"]);
  }
  if (tileJson.contains("isWalkable")) {
    meta.isWalkable = tileJson["isWalkable"].get<bool>();
  }
  if (tileJson.contains("isSeeThrough")) {
    meta.isSeeThrough = tileJson["isSeeThrough"].get<bool>();
  }
  if (tileJson.contains("isDoor")) {
    meta.isDoor = tileJson["isDoor"].get<bool>();
  }
  if (tileJson.contains("isContainer")) {
    meta.isContainer = tileJson["isContainer"].get<bool>();
  }
  return meta;
}

model::TilesetTemplate parseTileset(const Json& tilesetJson) {
  auto tileset = model::TilesetTemplate{};

  if (!tilesetJson.contains("name") || !tilesetJson["name"].is_string()) {
    throw std::runtime_error("Tileset template missing name");
  }
  tileset.name = tilesetJson["name"].get<bmin::String>();

  if (tilesetJson.contains("spriteBase") && tilesetJson["spriteBase"].is_string()) {
    tileset.spriteBase = tilesetJson["spriteBase"].get<bmin::String>();
  }
  if (tilesetJson.contains("tileWidth")) {
    tileset.tileWidth = tilesetJson["tileWidth"].get<int>();
  }
  if (tilesetJson.contains("tileHeight")) {
    tileset.tileHeight = tilesetJson["tileHeight"].get<int>();
  }

  if (tilesetJson.contains("tiles") && tilesetJson["tiles"].is_array()) {
    for (const auto& tileJson : tilesetJson["tiles"]) {
      tileset.tiles.pushBack(parseTileMetadata(tileJson));
    }
  }

  return tileset;
}

} // namespace

void loadTilesetTemplates(const bmin::String& tilesetsFilePath,
                          bmin::Map<bmin::String, model::TilesetTemplate>& tilesetTemplates) {
  const auto fileContent = sdl2w::loadFileAsString(bmin::toStringView(tilesetsFilePath));

  Json jsonData;
  try {
    jsonData = Json::parse(fileContent.cStr(), nullptr, true, true);
  } catch (const Json::parse_error& e) {
    throw std::runtime_error(
        (bmin::String("Failed to parse tilesets JSON: ") + e.what()).cStr());
  }

  if (!jsonData.is_array()) {
    throw std::runtime_error("Tilesets JSON must be an array");
  }

  for (const auto& tilesetJson : jsonData) {
    auto tileset = parseTileset(tilesetJson);
    if (tilesetTemplates.contains(tileset.name)) {
      throw std::runtime_error(
          (bmin::String("Duplicate tileset template name: ") + tileset.name).cStr());
    }
    tilesetTemplates[tileset.name] = std::move(tileset);
  }
}

} // namespace db
