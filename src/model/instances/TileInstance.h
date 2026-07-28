#pragma once

#include "game/map/TileFields.h"
#include "model/templates/Maps.h"
#include <optional>
#include "bmin/DynArray.h"
#include "bmin/String.h"

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
  bmin::DynArray<game::TileField> fields;
};

} // namespace model
