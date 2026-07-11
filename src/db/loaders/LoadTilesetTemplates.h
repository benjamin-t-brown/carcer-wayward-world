#pragma once

#include "bmin/String.h"
#include "lib/bmin/Map.h"
#include "model/templates/Tileset.h"

namespace db {

void loadTilesetTemplates(const bmin::String& tilesetsFilePath,
                          bmin::Map<bmin::String, model::TilesetTemplate>& tilesetTemplates);

} // namespace db
