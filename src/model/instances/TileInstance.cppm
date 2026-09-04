module;
#include <cstddef>
#include <cstdint>
#include <utility>
#include <optional>

export module carcer.model.instances.TileInstance;
export import bmin.containers;
import bmin.string_interop;
export import carcer.game.map.TileFields;
export import carcer.model.templates.Maps;

export {

// --- from model/instances/TileInstance.h ---
namespace model {

struct TileInstance {
  bmin::String id;
  bmin::String tilesetName;
  int tileId = 0;
  int x = 0;
  int y = 0;
  std::optional<TileOverrides> tileOverrides;
  std::optional<TileLightSource> lightSource;
  std::optional<TileEventTrigger> eventTrigger;
  std::optional<TravelTrigger> travelTrigger;
  bool isExplored = false;
  bool isVisible = false;
  bool isContainer = false;
  bool isWalkable = false;
  bmin::DynArray<game::TileField> fields;
};

} // namespace model

} // export
