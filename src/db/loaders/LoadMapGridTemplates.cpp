#include "LoadMapGridTemplates.h"
#include "bmin/StringInterop.h"
#include "lib/Json.h"
#include "sdl2w/AssetLoader.h"
#include <algorithm>
#include <stdexcept>

namespace db {
namespace {

bmin::DynArray<bmin::DynArray<bmin::String>> createEmptyCells(int gridWidth, int gridHeight) {
  const int width = std::max(1, gridWidth);
  const int height = std::max(1, gridHeight);
  bmin::DynArray<bmin::DynArray<bmin::String>> cells;
  cells.resize(static_cast<size_t>(height));
  for (int y = 0; y < height; ++y) {
    cells[static_cast<size_t>(y)].resize(static_cast<size_t>(width));
    for (int x = 0; x < width; ++x) {
      cells[static_cast<size_t>(y)][static_cast<size_t>(x)] = bmin::String();
    }
  }
  return cells;
}

bmin::DynArray<bmin::DynArray<bmin::String>>
resizeCells(const bmin::DynArray<bmin::DynArray<bmin::String>>& source,
            int gridWidth,
            int gridHeight) {
  auto next = createEmptyCells(gridWidth, gridHeight);
  const int copyHeight =
      std::min(static_cast<int>(source.size()), static_cast<int>(next.size()));
  for (int y = 0; y < copyHeight; ++y) {
    const auto& row = source[static_cast<size_t>(y)];
    const int copyWidth =
        std::min(static_cast<int>(row.size()), static_cast<int>(next[static_cast<size_t>(y)].size()));
    for (int x = 0; x < copyWidth; ++x) {
      next[static_cast<size_t>(y)][static_cast<size_t>(x)] = row[static_cast<size_t>(x)];
    }
  }
  return next;
}

model::MapGridTemplate parseMapGrid(const Json& gridJson) {
  model::MapGridTemplate grid;

  if (!gridJson.contains("name") || !gridJson["name"].is_string()) {
    throw std::runtime_error("Map grid template missing name");
  }
  grid.name = gridJson["name"].get<bmin::String>();

  if (gridJson.contains("label") && gridJson["label"].is_string()) {
    grid.label = gridJson["label"].get<bmin::String>();
  }
  if (gridJson.contains("gridWidth")) {
    grid.gridWidth = gridJson["gridWidth"].get<int>();
  }
  if (gridJson.contains("gridHeight")) {
    grid.gridHeight = gridJson["gridHeight"].get<int>();
  }
  if (gridJson.contains("mapWidth")) {
    grid.mapWidth = gridJson["mapWidth"].get<int>();
  }
  if (gridJson.contains("mapHeight")) {
    grid.mapHeight = gridJson["mapHeight"].get<int>();
  }

  grid.gridWidth = std::max(1, grid.gridWidth);
  grid.gridHeight = std::max(1, grid.gridHeight);
  grid.mapWidth = std::max(1, grid.mapWidth);
  grid.mapHeight = std::max(1, grid.mapHeight);

  bmin::DynArray<bmin::DynArray<bmin::String>> rawCells;
  if (gridJson.contains("cells") && gridJson["cells"].is_array()) {
    for (const auto& rowJson : gridJson["cells"]) {
      bmin::DynArray<bmin::String> row;
      if (rowJson.is_array()) {
        for (const auto& cellJson : rowJson) {
          if (cellJson.is_string()) {
            row.pushBack(cellJson.get<bmin::String>());
          } else {
            row.pushBack(bmin::String());
          }
        }
      }
      rawCells.pushBack(std::move(row));
    }
  }

  grid.cells = resizeCells(rawCells, grid.gridWidth, grid.gridHeight);
  return grid;
}

} // namespace

void loadMapGridTemplates(const bmin::String& mapGridsFilePath,
                          bmin::Map<bmin::String, model::MapGridTemplate>& mapGridTemplates) {
  const bmin::String fileContent = sdl2w::loadFileAsString(bmin::toStringView(mapGridsFilePath));

  Json jsonData;
  try {
    jsonData = Json::parse(fileContent.cStr(), nullptr, true, true);
  } catch (const Json::parse_error& e) {
    throw std::runtime_error(
        (bmin::String("Failed to parse map grids JSON: ") + e.what()).cStr());
  }

  if (!jsonData.is_array()) {
    throw std::runtime_error("Map grids JSON must be an array");
  }

  for (const auto& gridJson : jsonData) {
    model::MapGridTemplate grid = parseMapGrid(gridJson);
    if (mapGridTemplates.contains(grid.name)) {
      throw std::runtime_error(
          (bmin::String("Duplicate map grid template name: ") + grid.name).cStr());
    }
    mapGridTemplates[grid.name] = std::move(grid);
  }
}

} // namespace db
