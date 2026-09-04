module;
#include <cstddef>
#include <cstdint>
#include <utility>
#include <optional>
#if __has_include(<SDL.h>)
#include <SDL.h>
#include <SDL_pixels.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_pixels.h>
#endif
#include <stdexcept>
#include <algorithm>

export module carcer.ui.components.MapView;
export import bmin.containers;
import bmin.string_interop;
export import carcer.model.instances.World;
export import carcer.state;
export import carcer.ui.UiElement;
import sdl2w;
import carcer.game.map.TileFields;
import carcer.ui.colors;
import carcer.game.map;
import carcer.model.instances.CharacterInstance;
import carcer.model.instances.CharacterPlayer;
import carcer.model.instances.MapInstance;
import carcer.model.templates.CharacterTemplate;
import carcer.model.templates.Maps;
import carcer.model.templates.UtilityTypes;
import carcer.ui.FontScale;
#include "macros.h"

export {

// --- from ui/components/MapView.h ---
namespace ui {

struct MapViewProps {
  int width = 0;
  int height = 0;
};

// Draws State.world.activeMap tiles (from stitched MapInstances), items,
// characters, and damage particles into a clipped content rect using
// State.world.camera.camX / camY (map pixel space). Does not own or mutate camera.
class MapView : public UiElement, public state::DatabaseInterface {
private:
  MapViewProps props;

  SDL_Color mapFogColor{0, 0, 0, 128};
  SDL_Color mapUnexploredColor{0, 0, 0, 255};
  SDL_Color actionAimFillColor{66, 202, 253, 64};
  SDL_Color actionAimOutlineColor{66, 202, 253, 220};

  bmin::Map<bmin::String, bmin::UniquePtr<sdl2w::Animation>> animations;

  void renderDamageParticles(const model::World& world,
                             sdl2w::Draw& draw,
                             sdl2w::Store& store,
                             int contentX,
                             int contentY,
                             int spriteW,
                             int spriteH, 
                             int fontScale);

  void renderProjectiles(const model::World& world,
                         sdl2w::Draw& draw,
                         sdl2w::Store& store,
                         int contentX,
                         int contentY,
                         int spriteW,
                         int spriteH);

  void addAnimation(const bmin::String& animationName);
  sdl2w::Animation* getAnimation(const bmin::String& animationName);
  sdl2w::Animation* upsertAnimation(const bmin::String& animationName);

public:
  MapView(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~MapView() override = default;

  void setProps(const MapViewProps& _props);
  MapViewProps& getProps();
  const MapViewProps& getProps() const;

  // Screen pixel → map tile using the inverse of MapView render math.
  // nullopt if outside content rect or outside map bounds.
  std::optional<model::TileXY> screenToTile(int screenX, int screenY) const;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

namespace ui {

MapView::MapView(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {}

void MapView::setProps(const MapViewProps& _props) {
  props = _props;
  build();
}

MapViewProps& MapView::getProps() { return props; }

const MapViewProps& MapView::getProps() const { return props; }

std::optional<model::TileXY> MapView::screenToTile(int screenX, int screenY) const {
  auto* stateManager = getStateManager();
  if (!stateManager) {
    return std::nullopt;
  }

  const auto& world = stateManager->getState().world;
  if (world.activeMap.gridId.empty()) {
    return std::nullopt;
  }

  game::ActiveMapOrchestrator orch;
  try {
    orch.fetchMapGrid(world.activeMap.gridId);
  } catch (...) {
    return std::nullopt;
  }
  const auto total = orch.getTotalMapTilesSize();
  if (!total.valid || total.x <= 0 || total.y <= 0) {
    return std::nullopt;
  }

  auto contentX = style.x;
  auto contentY = style.y;
  auto contentW = static_cast<int>(style.width * style.scale);
  auto contentH = static_cast<int>(style.height * style.scale);
  if (contentW <= 0 || contentH <= 0) {
    return std::nullopt;
  }
  if (screenX < contentX || screenY < contentY || screenX >= contentX + contentW ||
      screenY >= contentY + contentH) {
    return std::nullopt;
  }

  auto* defaultMap = orch.getDefaultMapInstance();
  auto spriteW = defaultMap && defaultMap->spriteWidth > 0 ? defaultMap->spriteWidth : 28;
  auto spriteH =
      defaultMap && defaultMap->spriteHeight > 0 ? defaultMap->spriteHeight : 32;
  if (spriteW <= 0 || spriteH <= 0 || style.scale <= 0.f) {
    return std::nullopt;
  }

  const auto mapPx =
      static_cast<int>((screenX - contentX) / style.scale) + world.camera.camX;
  const auto mapPy =
      static_cast<int>((screenY - contentY) / style.scale) + world.camera.camY;
  const auto tileX = mapPx / spriteW;
  const auto tileY = mapPy / spriteH;
  if (tileX < 0 || tileY < 0 || tileX >= total.x || tileY >= total.y) {
    return std::nullopt;
  }
  return model::TileXY{tileX, tileY};
}

void MapView::build() {
  if (props.width > 0) {
    style.width = props.width;
  }
  if (props.height > 0) {
    style.height = props.height;
  }
}

void MapView::addAnimation(const bmin::String& animationName) {
  auto& store = window->getStore();
  auto anim = store.createAnimation(bmin::toStringView(animationName));
  animations.insert(animationName, bmin::makeUnique<sdl2w::Animation>(std::move(anim)));
}

sdl2w::Animation* MapView::getAnimation(const bmin::String& animationName) {
  for (auto& animation : animations) {
    if (animation.key == animationName) {
      return animation.value.get();
    }
  }
  return nullptr;
}

sdl2w::Animation* MapView::upsertAnimation(const bmin::String& animationName) {
  if (getAnimation(animationName)) {
    return getAnimation(animationName);
  }
  addAnimation(animationName);
  return getAnimation(animationName);
}

void MapView::renderDamageParticles(const model::World& world,
                                    sdl2w::Draw& draw,
                                    sdl2w::Store& store,
                                    int contentX,
                                    int contentY,
                                    int spriteW,
                                    int spriteH,
                                    int fontScale) {
  if (world.activeMap.damageParticles.empty() || style.scale <= 0.f ||
      world.activeMap.gridId.empty()) {
    return;
  }

  game::ActiveMapOrchestrator orch;
  orch.fetchMapGrid(world.activeMap.gridId);

  for (size_t i = 0; i < world.activeMap.damageParticles.size(); i++) {
    auto& particle = world.activeMap.damageParticles[i];
    auto* map = orch.getMapInstanceAt(particle.tileX, particle.tileY);
    const auto local = orch.activeMapCoordToInstanceCoord(particle.tileX, particle.tileY);
    if (!map || !local.valid) {
      continue;
    }
    map->tileLayerNumber = world.activeMap.mapLayer;
    if (!game::isTileCurrentlyVisible(*map, local.x, local.y)) {
      continue;
    }

    auto screenX =
        contentX +
        static_cast<int>((particle.tileX * spriteW - world.camera.camX) * style.scale);
    auto screenY =
        contentY +
        static_cast<int>((particle.tileY * spriteH - world.camera.camY) * style.scale);

    auto centerX = screenX + static_cast<int>(spriteW * style.scale / 2);
    auto centerY = screenY + static_cast<int>(spriteH * style.scale / 2);

    auto anim = upsertAnimation(particle.animationName);
    if (anim) {
      draw.drawAnimation(*anim,
                         sdl2w::RenderableParamsEx{
                             .scale = {style.scale, style.scale},
                             .x = centerX,
                             .y = centerY,
                             .centered = true,
                         });
    }

    if (!particle.text.empty()) {
      auto& damageText = particle.text;
      sdl2w::RenderTextParams textParams;
      textParams.fontName = "text-bold";
      textParams.fontSize = ui::applyFontScale(sdl2w::TEXT_SIZE_14, fontScale);
      textParams.x = centerX;
      textParams.y = centerY;
      textParams.color = Colors::White;
      textParams.centered = true;
      textParams.scale = {style.scale, style.scale};
      draw.drawText(bmin::toStringView(damageText), textParams);
    }
  }
}

void MapView::renderProjectiles(const model::World& world,
                                sdl2w::Draw& draw,
                                sdl2w::Store& store,
                                int contentX,
                                int contentY,
                                int spriteW,
                                int spriteH) {
  if (world.activeMap.projectiles.empty() || style.scale <= 0.f) {
    return;
  }

  for (size_t i = 0; i < world.activeMap.projectiles.size(); i++) {
    const auto& projectile = world.activeMap.projectiles[i];
    auto anim = upsertAnimation(projectile.animationName);
    if (!anim) {
      continue;
    }

    const auto pct = model::timerStructGetPct(projectile.travel);
    const auto tileX =
        projectile.fromTileX +
        (projectile.toTileX - projectile.fromTileX) * static_cast<float>(pct);
    const auto tileY =
        projectile.fromTileY +
        (projectile.toTileY - projectile.fromTileY) * static_cast<float>(pct);

    auto screenX =
        contentX +
        static_cast<int>((tileX * static_cast<float>(spriteW) - world.camera.camX) *
                         style.scale);
    auto screenY =
        contentY +
        static_cast<int>((tileY * static_cast<float>(spriteH) - world.camera.camY) *
                         style.scale);

    auto centerX = screenX + static_cast<int>(spriteW * style.scale / 2);
    auto centerY = screenY + static_cast<int>(spriteH * style.scale / 2) -
                   static_cast<int>(projectile.yOffset * style.scale);

    draw.drawAnimation(*anim,
                       sdl2w::RenderableParamsEx{
                           .scale = {style.scale, style.scale},
                           .x = centerX,
                           .y = centerY,
                           .centered = true,
                       });
  }
}

void MapView::render(int dt) {
  auto* stateManager = getStateManager();
  if (!stateManager) {
    return;
  }
  auto* database = getDatabase();
  if (!database) {
    return;
  }

  for (auto& animation : animations) {
    animation.value->update(dt);
  }

  auto& draw = window->getDraw();
  const auto& state = stateManager->getState();
  const auto& world = state.world;
  if (world.activeMap.gridId.empty()) {
    return;
  }

  game::ActiveMapOrchestrator orch;
  orch.fetchMapGrid(world.activeMap.gridId);

  const auto total = orch.getTotalMapTilesSize();
  if (!total.valid || total.x <= 0 || total.y <= 0) {
    return;
  }

  auto contentX = style.x;
  auto contentY = style.y;
  auto contentW = static_cast<int>(style.width * style.scale);
  auto contentH = static_cast<int>(style.height * style.scale);
  if (contentW <= 0 || contentH <= 0) {
    return;
  }

  auto* defaultMap = orch.getDefaultMapInstance();
  auto spriteW = defaultMap && defaultMap->spriteWidth > 0 ? defaultMap->spriteWidth : 28;
  auto spriteH =
      defaultMap && defaultMap->spriteHeight > 0 ? defaultMap->spriteHeight : 32;
  auto scaledSpriteW = static_cast<int>(spriteW * style.scale);
  auto scaledSpriteH = static_cast<int>(spriteH * style.scale);
  if (scaledSpriteW <= 0 || scaledSpriteH <= 0) {
    return;
  }

  auto& store = window->getStore();

  auto drawMapSprite =
      [&](sdl2w::Sprite& sprite, int screenX, int screenY, bool flipped = false) {
        if (screenX + scaledSpriteW <= contentX || screenX >= contentX + contentW ||
            screenY + scaledSpriteH <= contentY || screenY >= contentY + contentH) {
          return;
        }
        draw.drawSprite(sprite,
                        sdl2w::RenderableParamsEx{
                            .scale = {style.scale, style.scale},
                            .x = screenX,
                            .y = screenY,
                            .centered = false,
                            .flipped = flipped,
                        });
      };

  const int startTileX = std::max(0, world.camera.camX / spriteW - 1);
  const int startTileY = std::max(0, world.camera.camY / spriteH - 1);
  const int endTileX = std::min(
      total.x,
      (world.camera.camX + contentW / static_cast<int>(style.scale)) / spriteW + 2);
  const int endTileY = std::min(
      total.y,
      (world.camera.camY + contentH / static_cast<int>(style.scale)) / spriteH + 2);

  for (auto y = startTileY; y < endTileY; y++) {
    for (auto x = startTileX; x < endTileX; x++) {
      auto* map = orch.getMapInstanceAt(x, y);
      const auto local = orch.activeMapCoordToInstanceCoord(x, y);
      if (!map || !local.valid) {
        continue;
      }
      map->tileLayerNumber = world.activeMap.mapLayer;

      auto screenX =
          contentX + static_cast<int>((x * spriteW - world.camera.camX) * style.scale);
      auto screenY =
          contentY + static_cast<int>((y * spriteH - world.camera.camY) * style.scale);

      const auto* tile = game::resolveTileToRender(*map, local.x, local.y);
      if (!tile || !tile->isExplored) {
        if (screenX + scaledSpriteW > contentX && screenX < contentX + contentW &&
            screenY + scaledSpriteH > contentY && screenY < contentY + contentH) {
          draw.drawRect(
              screenX, screenY, scaledSpriteW, scaledSpriteH, mapUnexploredColor);
        }
        continue;
      }

      auto spriteName = tile->tilesetName + "_" + bmin::toString(tile->tileId);
      if (!store.sprites.contains(spriteName)) {
        continue;
      }

      auto& sprite = store.getSprite(bmin::toStringView(spriteName));
      drawMapSprite(sprite, screenX, screenY);

      if (tile->isVisible) {
        if (const auto* surfaceTile = game::tileAtCurrentLayer(*map, local.x, local.y)) {
          for (size_t fi = 0; fi < surfaceTile->fields.size(); fi++) {
            const auto& field = surfaceTile->fields[fi];
            const auto fieldSpriteName = game::tileFieldSpriteName(field);
            if (!store.sprites.contains(fieldSpriteName)) {
              continue;
            }
            auto& fieldSprite = store.getSprite(bmin::toStringView(fieldSpriteName));
            drawMapSprite(fieldSprite, screenX, screenY);
          }

          auto drawOverlay = [&](model::TileOverlayVisibility visibility) {
            const auto overlaySpriteName =
                model::tileOverlayVisibilitySpriteName(visibility);
            if (overlaySpriteName.empty() || !store.sprites.contains(overlaySpriteName)) {
              return;
            }
            auto& overlaySprite = store.getSprite(bmin::toStringView(overlaySpriteName));
            drawMapSprite(overlaySprite, screenX, screenY);
          };
          if (surfaceTile->eventTrigger) {
            drawOverlay(surfaceTile->eventTrigger->overlayVisibility);
          }
          if (surfaceTile->travelTrigger) {
            drawOverlay(surfaceTile->travelTrigger->overlayVisibility);
          }
        }
      }

      if (!tile->isVisible) {
        if (screenX + scaledSpriteW > contentX && screenX < contentX + contentW &&
            screenY + scaledSpriteH > contentY && screenY < contentY + contentH) {
          draw.drawRect(screenX, screenY, scaledSpriteW, scaledSpriteH, mapFogColor);
        }
      }
    }
  }

  for (size_t ii = 0; ii < world.activeMap.items.size(); ii++) {
    const auto& item = world.activeMap.items[ii];
    auto* map = orch.getMapInstanceAt(item.x, item.y);
    const auto local = orch.activeMapCoordToInstanceCoord(item.x, item.y);
    if (!map || !local.valid) {
      continue;
    }
    map->tileLayerNumber = world.activeMap.mapLayer;
    if (!game::isTileCurrentlyVisible(*map, local.x, local.y)) {
      continue;
    }
    // Items on container tiles are stored inside the container, not drawn on the ground.
    if (const auto* tile = game::tileAtCurrentLayer(*map, local.x, local.y);
        tile && tile->isContainer) {
      continue;
    }
    bmin::String spriteName;
    try {
      const auto& itemTemplate =
          database->getItemTemplate(bmin::toStringView(item.itemTemplateName));
      spriteName = itemTemplate.iconSpriteName;
    } catch (const std::exception&) {
      continue;
    }
    if (spriteName.empty() || !store.sprites.contains(spriteName)) {
      continue;
    }

    auto screenX =
        contentX + static_cast<int>((item.x * spriteW - world.camera.camX) * style.scale);
    auto screenY =
        contentY + static_cast<int>((item.y * spriteH - world.camera.camY) * style.scale);
    auto centerX = screenX + scaledSpriteW / 2;
    auto centerY = screenY + scaledSpriteH / 2;

    auto& sprite = store.getSprite(bmin::toStringView(spriteName));
    if (centerX + scaledSpriteW / 2 <= contentX ||
        centerX - scaledSpriteW / 2 >= contentX + contentW ||
        centerY + scaledSpriteH / 2 <= contentY ||
        centerY - scaledSpriteH / 2 >= contentY + contentH) {
      continue;
    }
    draw.drawSprite(sprite,
                    sdl2w::RenderableParamsEx{
                        .scale = {style.scale, style.scale},
                        .x = centerX,
                        .y = centerY,
                        .centered = true,
                    });
  }

  const auto& party = state.player.party;
  const bmin::String* activeCharacterId = nullptr;
  if (world.combat.active && !world.combat.activeCharacterId.empty()) {
    activeCharacterId = &world.combat.activeCharacterId;
  }

  auto drawCharacter = [&](const model::CharacterInstance& character) {
    auto* map = orch.getMapInstanceAt(character.x, character.y);
    const auto local = orch.activeMapCoordToInstanceCoord(character.x, character.y);
    if (!map || !local.valid) {
      return;
    }
    map->tileLayerNumber = world.activeMap.mapLayer;
    if (!game::isTileCurrentlyVisible(*map, local.x, local.y)) {
      return;
    }
    const model::CharacterPlayer* member = nullptr;
    for (size_t pi = 0; pi < party.size(); pi++) {
      if (party[pi].instanceId == character.id) {
        member = &party[pi];
        break;
      }
    }

    bmin::String spriteName;
    if (member) {
      spriteName = model::characterPlayerGetSpriteAtIndexOffset(
          *member, character.spriteIndexOffset);
    } else if (database) {
      try {
        const auto& characterTemplate =
            database->getCharacterTemplate(bmin::toStringView(character.templateName));
        spriteName = model::characterGetSpriteAtIndexOffset(characterTemplate,
                                                            character.spriteIndexOffset);
      } catch (const std::exception&) {
        return;
      }
    } else {
      return;
    }

    if (spriteName.empty() || !store.sprites.contains(spriteName)) {
      return;
    }

    auto screenX =
        contentX +
        static_cast<int>((character.x * spriteW - world.camera.camX) * style.scale);
    auto screenY =
        contentY +
        static_cast<int>((character.y * spriteH - world.camera.camY) * style.scale);

    auto& sprite = store.getSprite(bmin::toStringView(spriteName));
    drawMapSprite(sprite, screenX, screenY, model::isCharacterFacingLeft(character));
  };

  const model::CharacterInstance* activeCharacter = nullptr;
  for (size_t ci = 0; ci < world.activeMap.characters.size(); ci++) {
    const auto& character = world.activeMap.characters[ci];
    if (activeCharacterId != nullptr && character.id == *activeCharacterId) {
      activeCharacter = &character;
      continue;
    }
    drawCharacter(character);
  }
  // draw the active ch on the top
  if (activeCharacter != nullptr) {
    drawCharacter(*activeCharacter);
  }

  renderDamageParticles(
      world, draw, store, contentX, contentY, spriteW, spriteH, state.settings.fontScale);
  renderProjectiles(world, draw, store, contentX, contentY, spriteW, spriteH);

  if (world.actionMode != model::WorldActionMode::NONE && world.actionAimTile) {
    const auto aimX = world.actionAimTile->x;
    const auto aimY = world.actionAimTile->y;

    int zoneW = 1;
    int zoneH = 1;
    if (world.actionMode == model::WorldActionMode::SPELL &&
        !world.pendingSpellId.empty()) {
      auto* database = getDatabase();
      if (database != nullptr) {
        const auto* spell =
            database->findSpellTemplate(bmin::toStringView(world.pendingSpellId));
        if (spell != nullptr) {
          const auto* ability =
              database->findAbilityTemplate(bmin::toStringView(spell->abilityName));
          if (ability != nullptr) {
            zoneW = ability->targetSelect.zoneSize.x > 0
                        ? ability->targetSelect.zoneSize.x
                        : 1;
            zoneH = ability->targetSelect.zoneSize.y > 0
                        ? ability->targetSelect.zoneSize.y
                        : 1;
          }
        }
      }
    }

    for (int zy = 0; zy < zoneH; ++zy) {
      for (int zx = 0; zx < zoneW; ++zx) {
        const auto tileX = aimX + zx;
        const auto tileY = aimY + zy;
        const auto screenX =
            contentX +
            static_cast<int>((tileX * spriteW - world.camera.camX) * style.scale);
        const auto screenY =
            contentY +
            static_cast<int>((tileY * spriteH - world.camera.camY) * style.scale);

        if (screenX + scaledSpriteW > contentX && screenX < contentX + contentW &&
            screenY + scaledSpriteH > contentY && screenY < contentY + contentH) {
          draw.drawRect(
              screenX, screenY, scaledSpriteW, scaledSpriteH, actionAimFillColor);
          const auto border = 2;
          draw.drawRect(screenX, screenY, scaledSpriteW, border, actionAimOutlineColor);
          draw.drawRect(screenX,
                        screenY + scaledSpriteH - border,
                        scaledSpriteW,
                        border,
                        actionAimOutlineColor);
          draw.drawRect(screenX, screenY, border, scaledSpriteH, actionAimOutlineColor);
          draw.drawRect(screenX + scaledSpriteW - border,
                        screenY,
                        border,
                        scaledSpriteH,
                        actionAimOutlineColor);
        }
      }
    }
  }
}

} // namespace ui
