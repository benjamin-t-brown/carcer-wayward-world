module;
#include <cstddef>
#include <cstdint>
#include <utility>

export module carcer.model.templates:MapGrids;
export import bmin.containers;
import bmin.string_interop;

export {

// --- from model/templates/MapGrids.h ---
namespace model {

// Matches ceditor MapGridTemplate / assets/db/map-grids.json.
// cells is row-major [y][x]; empty string = unassigned slot.
struct MapGridTemplate {
  bmin::String name;
  bmin::String label;
  int gridWidth = 1;
  int gridHeight = 1;
  int mapWidth = 1;
  int mapHeight = 1;
  bmin::DynArray<bmin::DynArray<bmin::String>> cells;
};

} // namespace model

} // export
