#include "LoadDropTables.h"
#include "bmin/StringInterop.h"
#include "lib/Json.h"
#include "sdl2w/AssetLoader.h"
#include <stdexcept>

namespace db {

namespace {

int parseDropEntryWeight(const Json& entryJson) {
  if (!entryJson.contains("weight")) {
    return 1;
  }
  if (!entryJson["weight"].is_number_integer()) {
    return 1;
  }
  const int weight = entryJson["weight"].get<int>();
  return weight <= 0 ? 1 : weight;
}

int parseOptionalDropInt(const Json& entryJson, const char* key, int fallback) {
  if (!entryJson.contains(key)) {
    return fallback;
  }
  if (!entryJson[key].is_number_integer()) {
    throw std::runtime_error(
        (bmin::String("Drop table entry field must be integer: ") + key).cStr());
  }
  return entryJson[key].get<int>();
}

void clampDropRange(int& min, int& max) {
  if (min < 0) {
    min = 0;
  }
  if (max < min) {
    max = min;
  }
}

model::DropTableEntry parseDropTableEntry(const Json& entryJson,
                                          const bmin::String& tableName) {
  model::DropTableEntry entry;
  entry.weight = parseDropEntryWeight(entryJson);

  const bool hasItem =
      entryJson.contains("item") && entryJson["item"].is_string();
  const bool hasNested =
      entryJson.contains("dropTable") && entryJson["dropTable"].is_string();
  const bool hasGoldKeys =
      entryJson.contains("goldMin") || entryJson.contains("goldMax");
  const bool hasFoodKeys =
      entryJson.contains("foodMin") || entryJson.contains("foodMax");
  const bool nothingKind =
      entryJson.contains("nothing") && entryJson["nothing"].is_boolean() &&
      entryJson["nothing"].get<bool>();

  bmin::String itemStr;
  bmin::String nestedStr;
  if (hasItem) {
    itemStr = entryJson["item"].get<bmin::String>();
  }
  if (hasNested) {
    nestedStr = entryJson["dropTable"].get<bmin::String>();
  }

  const bool itemKind = !itemStr.empty();
  const bool nestedKind = !nestedStr.empty();
  const bool goldKind = hasGoldKeys;
  const bool foodKind = hasFoodKeys;

  const int kindCount = static_cast<int>(itemKind) + static_cast<int>(nestedKind) +
                        static_cast<int>(goldKind) + static_cast<int>(foodKind) +
                        static_cast<int>(nothingKind);
  if (kindCount != 1) {
    throw std::runtime_error(
        (bmin::String("Drop table entry in ") + tableName.cStr() +
         " must have exactly one of item, dropTable, gold range, food range, or "
         "nothing")
            .cStr());
  }

  if (nothingKind) {
    entry.isNothingEntry = true;
    return entry;
  }
  if (nestedKind) {
    entry.dropTable = nestedStr;
    return entry;
  }
  if (itemKind) {
    entry.item = itemStr;
    return entry;
  }
  if (goldKind) {
    entry.goldMin = parseOptionalDropInt(entryJson, "goldMin", 0);
    entry.goldMax = parseOptionalDropInt(entryJson, "goldMax", entry.goldMin);
    clampDropRange(entry.goldMin, entry.goldMax);
    entry.isGoldRangeEntry = true;
    return entry;
  }

  entry.foodMin = parseOptionalDropInt(entryJson, "foodMin", 0);
  entry.foodMax = parseOptionalDropInt(entryJson, "foodMax", entry.foodMin);
  clampDropRange(entry.foodMin, entry.foodMax);
  entry.isFoodRangeEntry = true;
  return entry;
}

} // namespace

void loadDropTables(const bmin::String& dropTablesFilePath,
                    bmin::Map<bmin::String, model::DropTableTemplate>& dropTables) {
  const bmin::String fileContent =
      sdl2w::loadFileAsString(bmin::toStringView(dropTablesFilePath));

  Json jsonData;
  try {
    jsonData = Json::parse(fileContent.cStr(), nullptr, true, true);
  } catch (const Json::parse_error& e) {
    throw std::runtime_error((bmin::String("Failed to parse JSON file ") +
                              dropTablesFilePath.cStr() + ": " + e.what())
                                 .cStr());
  }

  if (!jsonData.is_array()) {
    throw std::runtime_error("drop-tables.json must contain an array of drop tables");
  }

  for (const auto& tableJson : jsonData) {
    model::DropTableTemplate table;

    if (!tableJson.contains("name") || !tableJson["name"].is_string()) {
      throw std::runtime_error("Drop table missing required field: name");
    }
    table.name = tableJson["name"].get<bmin::String>();
    if (table.name.empty()) {
      throw std::runtime_error("Drop table name must be non-empty");
    }

    if (!tableJson.contains("label") || !tableJson["label"].is_string()) {
      throw std::runtime_error(
          (bmin::String("Drop table missing required field: label: ") + table.name.cStr())
              .cStr());
    }
    table.label = tableJson["label"].get<bmin::String>();

    if (!tableJson.contains("entries") || !tableJson["entries"].is_array()) {
      throw std::runtime_error(
          (bmin::String("Drop table missing required field: entries: ") + table.name.cStr())
              .cStr());
    }

    for (const auto& entryJson : tableJson["entries"]) {
      table.entries.pushBack(parseDropTableEntry(entryJson, table.name));
    }

    dropTables[table.name] = std::move(table);
  }
}

} // namespace db
