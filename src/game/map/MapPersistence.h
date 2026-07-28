#pragma once

#include "bmin/Map.h"
#include "bmin/String.h"
#include "db/Database.h"
#include "model/instances/MapInstance.h"
#include "model/instances/World.h"

namespace game {

// Capture live map deltas into persistent state (explored + open doors).
void flushMapInstance(const model::MapInstance& map,
                      model::PersistentMapState& persistent,
                      const db::Database& database);

// Apply persistent deltas onto a freshly created model::MapInstance.
void hydrateMapInstance(model::MapInstance& map,
                        const model::PersistentMapState& persistent,
                        const db::Database& database);

// Flush model::World.currentMap into mapsByTemplate[templateName].
void flushCurrentMapToPersistence(
    model::World& world,
    bmin::Map<bmin::String, model::PersistentMapState>& mapsByTemplate,
    const db::Database& database);

// Hydrate model::World.currentMap from mapsByTemplate if present.
void hydrateCurrentMapFromPersistence(
    model::World& world,
    bmin::Map<bmin::String, model::PersistentMapState>& mapsByTemplate,
    const db::Database& database);

// Flush current → create from template → hydrate → set currentMap and world name/camera.
void enterMap(model::World& world,
              bmin::Map<bmin::String, model::PersistentMapState>& mapsByTemplate,
              const bmin::String& templateName,
              const db::Database& database);

// Record a defeated map enemy so it stays gone when revisiting the template.
void markMapCharacterDefeated(
    model::World& world,
    bmin::Map<bmin::String, model::PersistentMapState>& mapsByTemplate,
    const model::CharacterInstance& character);

// Advance the global movement clock and age tile fields on all persisted maps.
void advanceWorldMovementTicks(
    model::World& world,
    bmin::Map<bmin::String, model::PersistentMapState>& mapsByTemplate,
    int steps,
    const db::Database& database);

} // namespace game
