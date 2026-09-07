module;
#include <cstddef>
#include <cstdint>
#include <utility>
#include <SDL.h>
#include <SDL_pixels.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_pixels.h>
#include <optional>
#include <stdexcept>
#include <algorithm>
#include <cmath>

export module carcer.ui.widgets.views;
export import bmin.containers;
export import carcer.ui.core;
export import carcer.model;
export import carcer.state;
import bmin.string_interop;
import sdl2w;
import carcer.game.map.TileFields;
import carcer.game.map;
import carcer.data;
import carcer.ui.widgets.primitives;
import carcer.ui.widgets.controls;
import carcer.actions;
#include "macros.h"

export {

// --- from ui/components/ChCompactInfo.h ---
// IWYU pragma: keep

namespace ui {

struct ChCompactInfoProps {
  bmin::String characterSpriteName;
  bmin::DynArray<bmin::String> statusEffectSpriteNames;

  SDL_Color spriteBgColor = Colors::OffWhite;
  SDL_Color spriteBorderColor1 = Colors::LightGrey;
  SDL_Color spriteBorderColor2 = Colors::White;
  int spriteBorderSize = 1;
  int spriteBoxSize = 36;

  int statusIconSize = 12;
  int numStatusColumns = 2;

  int hp = 0;
  int mana = 0;
  bool isSelected = false;
  int padding = 4;
  sdl2w::TextSize fontSize = sdl2w::TEXT_SIZE_18;
};

// ChCompactInfo component - renders a character sprite, status effect icons,
// and health/mana values in a bordered box.
class ChCompactInfo : public UiElement {
private:
  ChCompactInfoProps props;

  int fontHeight = 20;

  int getContentWidth() const;
  int getContentHeight() const;

public:
  ChCompactInfo(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~ChCompactInfo() override = default;

  void setProps(const ChCompactInfoProps& _props);
  ChCompactInfoProps& getProps();
  const ChCompactInfoProps& getProps() const;

  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

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

  auto& state = stateManager->getState();
  auto& world = state.world;
  if (world.activeMap.gridId.empty()) {
    return std::nullopt;
  }

  game::ActiveMapOrchestrator orch(world.activeMap, state.mapInstances, getDatabase());
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

  auto* stateManager = getStateManager();
  if (stateManager == nullptr) {
    return;
  }
  auto& state = stateManager->getState();
  game::ActiveMapOrchestrator orch(
      state.world.activeMap, state.mapInstances, getDatabase());
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
  auto& state = stateManager->getState();
  auto& world = state.world;
  if (world.activeMap.gridId.empty()) {
    return;
  }

  game::ActiveMapOrchestrator orch(world.activeMap, state.mapInstances, database);
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
      if (!store.hasSprite(bmin::toStringView(spriteName))) {
        continue;
      }

      auto& sprite = store.getSprite(bmin::toStringView(spriteName));
      drawMapSprite(sprite, screenX, screenY);

      if (tile->isVisible) {
        if (const auto* surfaceTile = game::tileAtCurrentLayer(*map, local.x, local.y)) {
          for (size_t fi = 0; fi < surfaceTile->fields.size(); fi++) {
            const auto& field = surfaceTile->fields[fi];
            const auto fieldSpriteName = game::tileFieldSpriteName(field);
            if (!store.hasSprite(bmin::toStringView(fieldSpriteName))) {
              continue;
            }
            auto& fieldSprite = store.getSprite(bmin::toStringView(fieldSpriteName));
            drawMapSprite(fieldSprite, screenX, screenY);
          }

          auto drawOverlay = [&](model::TileOverlayVisibility visibility) {
            const auto overlaySpriteName =
                model::tileOverlayVisibilitySpriteName(visibility);
            if (overlaySpriteName.empty() ||
                !store.hasSprite(bmin::toStringView(overlaySpriteName))) {
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
    if (spriteName.empty() || !store.hasSprite(bmin::toStringView(spriteName))) {
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

    if (spriteName.empty() || !store.hasSprite(bmin::toStringView(spriteName))) {
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

export {

// --- from ui/components/borders/BorderDropShadow.h ---
namespace ui {

struct BorderDropShadowProps {
  int width = 0;
  int height = 0;
  SDL_Color backgroundColor = Colors::White;
  SDL_Color shadowColor = Colors::Black;
  int shadowOffsetX = -8;
  int shadowOffsetY = 8;
  int borderSize = 2;
  bool isSelected = false;
};

// BorderDropShadow - panel with fill, offset drop shadow, and outline border.
// Content children are laid out in logical space inside an internal scaled Quad.
class BorderDropShadow : public UiElement {
  BorderDropShadowProps props;

public:
  BorderDropShadow(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~BorderDropShadow() override = default;

  void setProps(const BorderDropShadowProps& _props);
  BorderDropShadowProps& getProps();
  const BorderDropShadowProps& getProps() const;

  void addChild(UiElement* child) override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

namespace ui {

BorderDropShadow::BorderDropShadow(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  shouldPropagateEventsToChildren = true;
}

void BorderDropShadow::setProps(const BorderDropShadowProps& _props) {
  props = _props;
  if (props.width > 0) {
    style.width = props.width;
  }
  if (props.height > 0) {
    style.height = props.height;
  }
  build();
}

BorderDropShadowProps& BorderDropShadow::getProps() { return props; }

const BorderDropShadowProps& BorderDropShadow::getProps() const { return props; }

void BorderDropShadow::addChild(UiElement* child) {
  if (!children.empty()) {
    children[0]->addChild(child);
  }
}

void BorderDropShadow::build() {
  bmin::DynArray<bmin::UniquePtr<UiElement>> preservedChildren;
  if (!children.empty()) {
    for (auto& child : children[0]->getChildren()) {
      preservedChildren.pushBack(std::move(child));
    }
  }

  if (props.width > 0) {
    style.width = props.width;
  }
  if (props.height > 0) {
    style.height = props.height;
  }

  auto quad = bmin::makeUnique<Quad>(window);
  quad->setPos(style.x, style.y);
  quad->setScale(style.scale);
  quad->setProps(QuadProps{
      .width = style.width,
      .height = style.height,
      .bgColor = props.backgroundColor,
      .borderColor = Colors::Transparent,
      .borderSize = 0,
  });

  for (auto& child : preservedChildren) {
    quad->addChild(child.release());
  }

  children.clear();
  children.pushBack(bmin::UniquePtr<UiElement>(quad.release()));
}

void BorderDropShadow::render(int dt) {
  auto& draw = window->getDraw();

  const int scaledWidth = static_cast<int>(style.width * style.scale);
  const int scaledHeight = static_cast<int>(style.height * style.scale);

  const int shadowX = style.x + props.shadowOffsetX;
  const int shadowY = style.y + props.shadowOffsetY;
  draw.drawRect(shadowX, shadowY, scaledWidth, scaledHeight, props.shadowColor);

  if (props.borderSize > 0) {
    draw.drawRect(style.x - props.borderSize,
                  style.y - props.borderSize,
                  scaledWidth + 2 * props.borderSize,
                  scaledHeight + 2 * props.borderSize,
                  props.shadowColor);
  }

  UiElement::render(dt);
}

} // namespace ui

export {

// --- from ui/components/borders/BorderInGame.h ---
namespace ui {

struct BorderInGameProps {
  int titleHeight = 44;
  int outsetBorderSize = 4;
  float actionButtonsScale = 1.f;
};

// BorderInGame component - base class for in-game border layouts using OutsetRectangle
// elements. Uses Position, Size, Scale from BaseStyle
class BorderInGame : public UiElement {
public:
  static constexpr int ACTION_BUTTON_SIZE = 32;

protected:
  virtual const BorderInGameProps& inGameProps() const = 0;

  int scaledWidth() const;
  int scaledHeight() const;
  void addOutsetRect(int x, int y, int width, int height);

public:
  BorderInGame(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~BorderInGame() override = default;

  const std::pair<int, int> getTitleLocation() const;
  const std::pair<int, int> getTitleDims() const;

  virtual const std::pair<int, int> getContentAreaLocation() const = 0;
  virtual const std::pair<int, int> getContentDims() const = 0;
  virtual const std::pair<int, int> getPartyMemberAreaLocation() const = 0;
  virtual const std::pair<int, int> getActionButtonsAreaLocation() const = 0;

  void render(int dt) override;
};

} // namespace ui

} // export

namespace ui {

BorderInGame::BorderInGame(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {}

int BorderInGame::scaledWidth() const {
  return static_cast<int>(style.width * style.scale);
}

int BorderInGame::scaledHeight() const {
  return static_cast<int>(style.height * style.scale);
}

void BorderInGame::addOutsetRect(int x, int y, int width, int height) {
  auto rectangle = new OutsetRectangle(window, this);
  rectangle->setPos(x, y);
  rectangle->setScale(style.scale);
  rectangle->setProps(OutsetRectangleProps{
      .width = width,
      .height = height,
      .borderSize = inGameProps().outsetBorderSize,
  });
  addChild(rectangle);
}

const std::pair<int, int> BorderInGame::getTitleLocation() const {
  const auto& props = inGameProps();
  int titleX = style.x + props.outsetBorderSize * style.scale;
  int titleY = style.y + props.outsetBorderSize * style.scale;
  return {titleX, titleY};
}

const std::pair<int, int> BorderInGame::getTitleDims() const {
  const auto& props = inGameProps();
  return {style.width * style.scale - props.outsetBorderSize * 2 * style.scale,
          props.titleHeight * style.scale - props.outsetBorderSize * 2 * style.scale};
}

void BorderInGame::render(int dt) { UiElement::render(dt); }

} // namespace ui

export {

// --- from ui/components/InGameTitleBar.h ---
namespace ui {

struct InGameTitleBarProps {
  int width = 0;
  int height = 0;
  bmin::String title = "Title";
  int day = 0;
  int food = 0;
  int ap = 0;
  bool showAp = false;

  int buttonSize = 32;
  int buttonSpacing = 0;
  int sectionSpacing = 12;
  int statSpacing = 16;
};

// InGameTitleBar component - title bar for the in-game layout with menu buttons,
// customizable title text, and day/food/ap stats.
class InGameTitleBar : public UiElement {
private:
  InGameTitleBarProps props;

  int getContentHeight() const;
  int getBarCenterY() const;
  int centerTopY(int elementHeight) const;

public:
  InGameTitleBar(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~InGameTitleBar() override = default;

  void setProps(const InGameTitleBarProps& _props);
  InGameTitleBarProps& getProps();
  const InGameTitleBarProps& getProps() const;

  // void addMenuButtonObserver(UiEventObserver* observer);
  // void addHelpButtonObserver(UiEventObserver* observer);

  UiElement* createStatLineRightAligned(const bmin::String& text, int x, int y);

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

namespace ui {

InGameTitleBar::InGameTitleBar(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {}

void InGameTitleBar::setProps(const InGameTitleBarProps& _props) {
  props = _props;
  if (props.width > 0) {
    style.width = props.width;
  }
  if (props.height > 0) {
    style.height = props.height;
  }
  build();
}

InGameTitleBarProps& InGameTitleBar::getProps() { return props; }

const InGameTitleBarProps& InGameTitleBar::getProps() const { return props; }

int InGameTitleBar::getContentHeight() const { return props.buttonSize; }

int InGameTitleBar::getBarCenterY() const {
  return style.y + static_cast<int>(style.height * style.scale) / 2;
}

int InGameTitleBar::centerTopY(int elementHeight) const {
  return getBarCenterY() - elementHeight / 2;
}

UiElement*
InGameTitleBar::createStatLineRightAligned(const bmin::String& text, int x, int y) {
  auto statLine = new TextLine(window, this);
  TextFontProps font;
  setBaseFontConfig(font, BaseFontConfig::MODAL_TEXT_BOLD);
  TextLineProps statLineProps;
  statLineProps.fontFamily = font.fontFamily;
  statLineProps.fontSize = font.fontSize;
  statLineProps.fontColor = font.fontColor;
  statLineProps.textAlign = TextAlign::LEFT_CENTER;
  statLineProps.textBlocks.pushBack(TextBlock{.text = text});
  statLine->setPos(style.x, style.y);
  statLine->setScale(1.f);
  statLine->setProps(statLineProps);
  auto [statWidth, _] = statLine->getDims();
  statLine->setPos(x - statWidth, y);
  return statLine;
}

void InGameTitleBar::build() {
  children.clear();

  if (props.width > 0) {
    style.width = props.width;
  }
  if (props.height > 0) {
    style.height = props.height;
  }

  const int scaledButtonSize = static_cast<int>(props.buttonSize * style.scale);
  const int scaledButtonSpacing = static_cast<int>(props.buttonSpacing * style.scale);
  const int scaledSectionSpacing = static_cast<int>(props.sectionSpacing * style.scale);
  const int scaledStatSpacing = static_cast<int>(props.statSpacing * style.scale);
  const int barCenterY = getBarCenterY();
  const int buttonTopY = centerTopY(scaledButtonSize);
  auto [scaledWidth, scaledHeight] = getDims();

  int cursorX = style.x;

  auto menuButton = new ButtonIcon(window, this);
  menuButton->setId("menuButton");
  menuButton->setPos(cursorX, buttonTopY);
  menuButton->setScale(style.scale);
  menuButton->setProps(ButtonIconProps{
      .regularSprite = ButtonIcon::HAMBURGER_ICON1,
      .activeSprite = ButtonIcon::HAMBURGER_ICON2,
      .iconSize = props.buttonSize,
  });
  addChild(menuButton);
  cursorX += scaledButtonSize + scaledButtonSpacing;

  auto helpButton = new ButtonIcon(window, this);
  helpButton->setId("helpButton");
  helpButton->setPos(cursorX, buttonTopY);
  helpButton->setScale(style.scale);
  helpButton->setProps(ButtonIconProps{
      .regularSprite = ButtonIcon::QUESTION_ICON1,
      .activeSprite = ButtonIcon::QUESTION_ICON2,
      .iconSize = props.buttonSize,
  });
  addChild(helpButton);
  cursorX += scaledButtonSize + scaledSectionSpacing;

  TextFontProps titleFont;
  setBaseFontConfig(titleFont, BaseFontConfig::MODAL_TITLE);

  auto titleText = new TextLine(window, this);
  titleText->setId("titleText");
  titleText->setPos(cursorX, barCenterY);
  titleText->setScale(1.f);
  TextLineProps titleProps;
  titleProps.fontFamily = titleFont.fontFamily;
  titleProps.fontSize = titleFont.fontSize;
  titleProps.fontColor = titleFont.fontColor;
  titleProps.textAlign = TextAlign::LEFT_CENTER;
  titleProps.textBlocks.pushBack(TextBlock{.text = props.title});
  titleText->setProps(titleProps);
  addChild(titleText);

  int statX = style.x + scaledWidth - scaledStatSpacing;
  auto* dayStatLine = createStatLineRightAligned(
      TRANSLATE("Day: ") + bmin::toString(props.day), statX, barCenterY);
  addChild(dayStatLine);
  auto [dayStatWidth, _] = dayStatLine->getDims();
  statX -= dayStatWidth + scaledStatSpacing;
  auto* foodStatLine = createStatLineRightAligned(
      TRANSLATE("Food: ") + bmin::toString(props.food), statX, barCenterY);
  addChild(foodStatLine);
  if (props.showAp) {
    auto [foodStatWidth, _] = foodStatLine->getDims();
    statX -= foodStatWidth + scaledStatSpacing;
    auto* apStatLine = createStatLineRightAligned(
        TRANSLATE("AP: ") + bmin::toString(props.ap), statX, barCenterY);
    addChild(apStatLine);
  }
}

void InGameTitleBar::render(int dt) { UiElement::render(dt); }

} // namespace ui

export {

// --- from ui/components/ItemInfo.h ---
namespace ui {

struct ItemInfoProps {
  int width = 0;
  bmin::String description;
  int weight = 0;
  int value = 0;
};

// ItemInfo - description, weight, and value for an item template.
class ItemInfo : public UiElement {
private:
  ItemInfoProps props;

  static constexpr int kVertSpacerHeight = 12;

public:
  ItemInfo(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~ItemInfo() override = default;

  void setProps(const ItemInfoProps& _props);
  const ItemInfoProps& getProps() const;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

namespace ui {

ItemInfo::ItemInfo(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {}

void ItemInfo::setProps(const ItemInfoProps& _props) {
  props = _props;
  if (props.width > 0) {
    style.width = props.width;
  }
  build();
}

const ItemInfoProps& ItemInfo::getProps() const { return props; }

void ItemInfo::build() {
  children.clear();

  if (props.width > 0) {
    style.width = props.width;
  }

  TextFontProps font;
  setBaseFontConfig(font, BaseFontConfig::MODAL_TEXT);

  auto description = new TextParagraph(window, this);
  description->setId("description");
  description->setPos(style.x, style.y);
  description->setScale(1.f);
  TextParagraphProps descProps;
  descProps.width = style.width;
  descProps.fontFamily = font.fontFamily;
  descProps.fontSize = font.fontSize;
  descProps.fontColor = Colors::Black;
  descProps.textAlign = TextAlign::LEFT_TOP;
  descProps.lineSpacing = 0;
  descProps.textBlocks.pushBack({.text = props.description});
  description->setProps(descProps);
  addChild(description);

  auto [descWidthScaled, descHeightScaled] = description->getDims();
  auto weightLine = new TextLine(window, this);
  weightLine->setId("weight");
  weightLine->setPos(style.x,
                     style.y + descHeightScaled +
                         static_cast<int>(kVertSpacerHeight * style.scale));
  weightLine->setScale(1.f);
  TextLineProps weightProps;
  weightProps.fontFamily = font.fontFamily;
  weightProps.fontSize = font.fontSize;
  weightProps.fontColor = Colors::DarkBlue;
  weightProps.textAlign = TextAlign::LEFT_TOP;
  weightProps.textBlocks.pushBack({
      .text = TRANSLATE("Weight: ") + bmin::toString(props.weight) +
              bmin::String(TRANSLATE(" lbs")),
  });
  weightLine->setProps(weightProps);
  addChild(weightLine);
  const auto weightLineHeight = weightLine->getDims().second;

  auto valueLine = new TextLine(window, this);
  valueLine->setId("value");
  valueLine->setPos(style.x,
                    style.y + descHeightScaled +
                        static_cast<int>(kVertSpacerHeight * style.scale) +
                        weightLineHeight +
                        static_cast<int>(kVertSpacerHeight * style.scale));
  valueLine->setScale(1.f);
  TextLineProps valueProps;
  valueProps.fontFamily = font.fontFamily;
  valueProps.fontSize = font.fontSize;
  valueProps.fontColor = Colors::DarkGrey;
  valueProps.textAlign = TextAlign::LEFT_TOP;
  valueProps.textBlocks.pushBack({
      .text = TRANSLATE("Value: ") + bmin::toString(props.value) +
              bmin::String(TRANSLATE(" gp")),
  });
  valueLine->setProps(valueProps);
  addChild(valueLine);

  const auto valueLineHeight = valueLine->getDims().second;
  const int contentHeightPx = descHeightScaled +
                              static_cast<int>(kVertSpacerHeight * style.scale) +
                              weightLineHeight +
                              static_cast<int>(kVertSpacerHeight * style.scale) +
                              valueLineHeight;
  style.height = std::max(1, static_cast<int>(std::ceil(contentHeightPx / style.scale)));
}

void ItemInfo::render(int dt) { UiElement::render(dt); }

} // namespace ui

export {

// --- from ui/components/PartyMemberIconSelector.h ---
// IWYU pragma: keep

namespace ui {

enum class PartyMemberIconSelectorTarget { INVENTORY, PICKUP, MAGIC };

struct PartyMemberIconSelectorProps {
  bmin::DynArray<bmin::String> members;
  int selectedIndex = 0;
  PartyMemberIconSelectorTarget target = PartyMemberIconSelectorTarget::INVENTORY;
  int iconSize = 32;
  int iconGap = 4;

  SDL_Color spriteBgColor = Colors::LightGrey;
  SDL_Color spriteBgColorTopRight = Colors::White;
  SDL_Color spriteBgColorBottomLeft = Colors::ButtonModalGrey2;

  SDL_Color spriteSelectedBgColor = Colors::ButtonModalSelected;
  SDL_Color spriteSelectedBgColorTopRight = Colors::ButtonModalSelected;
  SDL_Color spriteSelectedBgColorBottomLeft = Colors::ButtonModalSelected;
};

class PartyMemberIconSelector : public UiElement {
private:
  PartyMemberIconSelectorProps props;

public:
  PartyMemberIconSelector(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~PartyMemberIconSelector() override = default;

  void setProps(const PartyMemberIconSelectorProps& _props);
  PartyMemberIconSelectorProps& getProps();
  const PartyMemberIconSelectorProps& getProps() const;

  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

// --- from ui/observers/ObserverSetCurrentPartyMember.hpp ---
namespace ui {

class ObserverSetCurrentPartyMember : public ui::UiEventObserver,
                                      public state::StateManagerInterface {
  int partyMemberIndex;

public:
  explicit ObserverSetCurrentPartyMember(int _partyMemberIndex)
      : partyMemberIndex(_partyMemberIndex) {}

  void onClick(int mouseX, int mouseY, int button) override;
};

} // namespace ui

// --- from ui/observers/ObserverSetCurrentPartyMemberInventory.hpp ---
namespace ui {

class ObserverSetCurrentPartyMemberInventory : public ui::UiEventObserver,
                                               public state::StateManagerInterface {
  int partyMemberInventoryIndex;

public:
  explicit ObserverSetCurrentPartyMemberInventory(int _partyMemberInventoryIndex)
      : partyMemberInventoryIndex(_partyMemberInventoryIndex) {}

  void onClick(int mouseX, int mouseY, int button) override;
};

} // namespace ui

// --- from ui/observers/ObserverSetCurrentPartyMemberMagic.hpp ---
namespace ui {

class ObserverSetCurrentPartyMemberMagic : public ui::UiEventObserver,
                                           public state::StateManagerInterface {
  int partyMemberMagicIndex;

public:
  explicit ObserverSetCurrentPartyMemberMagic(int _partyMemberMagicIndex)
      : partyMemberMagicIndex(_partyMemberMagicIndex) {}

  void onClick(int mouseX, int mouseY, int button) override;
};

} // namespace ui

} // export

namespace ui {

PartyMemberIconSelector::PartyMemberIconSelector(sdl2w::Window* _window,
                                                 UiElement* _parent)
    : UiElement(_window, _parent) {}

void PartyMemberIconSelector::setProps(const PartyMemberIconSelectorProps& _props) {
  props = _props;
  build();
}

PartyMemberIconSelectorProps& PartyMemberIconSelector::getProps() { return props; }

const PartyMemberIconSelectorProps& PartyMemberIconSelector::getProps() const {
  return props;
}

const std::pair<int, int> PartyMemberIconSelector::getDims() const {
  if (children.empty()) {
    if (props.members.empty()) {
      return {0, static_cast<int>(props.iconSize * style.scale)};
    }
    const int memberCount = static_cast<int>(props.members.size());
    const int w = memberCount * props.iconSize + (memberCount - 1) * props.iconGap;
    return {static_cast<int>(w * style.scale), static_cast<int>(props.iconSize * style.scale)};
  }
  return children[0]->getDims();
}

void PartyMemberIconSelector::build() {
  children.clear();

  if (props.members.empty()) {
    return;
  }

  ButtonGroupProps groupProps;
  groupProps.alignment = ButtonGroupAlignment::LEFT;
  groupProps.padding = 0;
  groupProps.buttonSpacing = props.iconGap;
  groupProps.buttonWidth = props.iconSize;
  groupProps.buttonHeight = props.iconSize;
  groupProps.spriteBgColor = props.spriteBgColor;
  groupProps.spriteBgColorTopRight = props.spriteBgColorTopRight;
  groupProps.spriteBgColorBottomLeft = props.spriteBgColorBottomLeft;
  groupProps.spriteSelectedBgColor = props.spriteSelectedBgColor;
  groupProps.spriteSelectedBgColorTopRight = props.spriteSelectedBgColorTopRight;
  groupProps.spriteSelectedBgColorBottomLeft = props.spriteSelectedBgColorBottomLeft;

  for (size_t i = 0; i < props.members.size(); ++i) {
    groupProps.buttons.pushBack(ButtonGroupButtonProps{
        .type = ButtonGroupButtonType::SPRITE,
        .spriteName = props.members[i],
        .spriteWidth = props.iconSize,
        .spriteHeight = props.iconSize,
        .spritePadding = 0,
        .isSelected = static_cast<int>(i) == props.selectedIndex,
    });
  }

  auto buttonGroup = new ButtonGroup(window, this);
  buttonGroup->setId("partyMemberButtonGroup");
  buttonGroup->setPos(style.x, style.y);
  buttonGroup->setScale(style.scale);
  buttonGroup->setProps(groupProps);

  for (size_t i = 0; i < props.members.size(); ++i) {
    if (props.target == PartyMemberIconSelectorTarget::PICKUP) {
      buttonGroup->addObserverToButtonAtIndex(static_cast<int>(i),
                                              new ObserverSetCurrentPartyMember(
                                                  static_cast<int>(i)));
    } else if (props.target == PartyMemberIconSelectorTarget::MAGIC) {
      buttonGroup->addObserverToButtonAtIndex(static_cast<int>(i),
                                              new ObserverSetCurrentPartyMemberMagic(
                                                  static_cast<int>(i)));
    } else {
      buttonGroup->addObserverToButtonAtIndex(static_cast<int>(i),
                                              new ObserverSetCurrentPartyMemberInventory(
                                                  static_cast<int>(i)));
    }
  }

  addChild(buttonGroup);
}

void PartyMemberIconSelector::render(int dt) { UiElement::render(dt); }

} // namespace ui

namespace ui {

void ObserverSetCurrentPartyMember::onClick(int mouseX, int mouseY, int button) {
    LOG(INFO) << "ObserverSetCurrentPartyMember::onClick index=" << partyMemberIndex
              << LOG_ENDL;
    auto stateManager = getStateManager();
    if (!stateManager) {
      return;
    }
    stateManager->enqueueAction(stateManager->getActionData(),
                                state::actions::setCurrentPartyMember(partyMemberIndex),
                                0);
  }

void ObserverSetCurrentPartyMemberInventory::onClick(int mouseX, int mouseY, int button) {
    LOG(INFO) << "ObserverSetCurrentPartyMemberInventory::onClick index="
              << partyMemberInventoryIndex << LOG_ENDL;
    auto stateManager = getStateManager();
    if (!stateManager) {
      return;
    }
    stateManager->enqueueAction(
        stateManager->getActionData(),
        state::actions::setCurrentPartyMemberInventory(partyMemberInventoryIndex),
        0);
  }

void ObserverSetCurrentPartyMemberMagic::onClick(int mouseX, int mouseY, int button) {
    LOG(INFO) << "ObserverSetCurrentPartyMemberMagic::onClick index="
              << partyMemberMagicIndex << LOG_ENDL;
    auto stateManager = getStateManager();
    if (!stateManager) {
      return;
    }
    stateManager->enqueueAction(
        stateManager->getActionData(),
        state::actions::setCurrentPartyMemberMagic(partyMemberMagicIndex),
        0);
  }

} // namespace ui

export {

// --- from ui/components/PartyMemberSwitcher.h ---
// IWYU pragma: keep

namespace ui {

struct PartyMemberSwitcherProps {
  bmin::String spriteName;
  int partyMemberIndex = 0;

  SDL_Color spriteBgColor = Colors::OffWhite;
  SDL_Color spriteBorderColor1 = Colors::LightGrey;
  SDL_Color spriteBorderColor2 = Colors::White;
  int spriteBorderSize = 1;
  int spriteBoxSize = 36;

  int buttonSize = 36;
  int buttonSpacing = 2;
};

// PartyMemberSwitcher - prev/next buttons flanking a party member sprite.
class PartyMemberSwitcher : public UiElement {
private:
  PartyMemberSwitcherProps props;

public:
  PartyMemberSwitcher(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~PartyMemberSwitcher() override = default;

  void setProps(const PartyMemberSwitcherProps& _props);
  PartyMemberSwitcherProps& getProps();
  const PartyMemberSwitcherProps& getProps() const;

  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

// --- from ui/observers/ObserverUpdateCurrentPartyMember.hpp ---
namespace ui {

class ObserverUpdateCurrentPartyMember : public ui::UiEventObserver,
                                         public state::StateManagerInterface {
  int directionDelta;

public:
  ObserverUpdateCurrentPartyMember(int /*_partyMemberIndex*/, int _directionDelta)
      : directionDelta(_directionDelta) {}

  void onClick(int mouseX, int mouseY, int button) override;
};

} // namespace ui

} // export

namespace ui {

PartyMemberSwitcher::PartyMemberSwitcher(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {}

void PartyMemberSwitcher::setProps(const PartyMemberSwitcherProps& _props) {
  props = _props;
  build();
}

PartyMemberSwitcherProps& PartyMemberSwitcher::getProps() { return props; }

const PartyMemberSwitcherProps& PartyMemberSwitcher::getProps() const { return props; }

const std::pair<int, int> PartyMemberSwitcher::getDims() const {
  const int w = props.buttonSize + props.buttonSpacing + props.spriteBoxSize +
                props.buttonSpacing + props.buttonSize;
  const int h = std::max(props.buttonSize, props.spriteBoxSize);
  return {static_cast<int>(w * style.scale), static_cast<int>(h * style.scale)};
}

void PartyMemberSwitcher::build() {
  children.clear();

  const int buttonSize = props.buttonSize;
  const int spriteSize = props.spriteBoxSize;
  const int spacing = props.buttonSpacing;
  const int totalWidth = buttonSize + spacing + spriteSize + spacing + buttonSize;
  const int totalHeight = std::max(buttonSize, spriteSize);

  style.width = totalWidth;
  style.height = totalHeight;

  auto container = new Quad(window, this);
  container->setPos(style.x, style.y);
  container->setScale(style.scale);
  container->setProps(QuadProps{
      .width = totalWidth,
      .height = totalHeight,
  });
  addChild(container);

  auto leftBtn = new ButtonModal(window, container);
  leftBtn->setId("prevPartyMember");
  leftBtn->setPos(0, (totalHeight - buttonSize) / 2);
  leftBtn->setProps(ButtonModalProps{.text = "<", .width = buttonSize, .height = buttonSize});
  leftBtn->addEventObserver(
      new ObserverUpdateCurrentPartyMember(props.partyMemberIndex, -1));
  container->addChild(leftBtn);

  const int spriteX = buttonSize + spacing;

  auto spriteRect = new OutsetRectangle(window, container);
  spriteRect->setPos(spriteX, 0);
  spriteRect->setProps(OutsetRectangleProps{
      .width = spriteSize,
      .height = spriteSize,
      .color = props.spriteBgColor,
      .colorTopRight = props.spriteBorderColor1,
      .colorBottomLeft = props.spriteBorderColor2,
      .borderSize = props.spriteBorderSize,
  });
  container->addChild(spriteRect);

  auto sprite = new Quad(window, container);
  sprite->setPos(spriteX + 1, 1);
  sprite->setProps(QuadProps{
      .width = spriteSize,
      .height = spriteSize,
      .bgSprite = props.spriteName,
  });
  container->addChild(sprite);

  auto rightBtn = new ButtonModal(window, container);
  rightBtn->setId("nextPartyMember");
  rightBtn->setPos(spriteX + spriteSize + spacing, (totalHeight - buttonSize) / 2);
  rightBtn->setProps(ButtonModalProps{.text = ">", .width = buttonSize, .height = buttonSize});
  rightBtn->addEventObserver(
      new ObserverUpdateCurrentPartyMember(props.partyMemberIndex, 1));
  container->addChild(rightBtn);
}

void PartyMemberSwitcher::render(int dt) { UiElement::render(dt); }

} // namespace ui

namespace ui {

void ObserverUpdateCurrentPartyMember::onClick(int mouseX, int mouseY, int button) {
    LOG(INFO) << "ObserverUpdateCurrentPartyMember::onClick delta=" << directionDelta
              << LOG_ENDL;
    auto stateManager = getStateManager();
    if (!stateManager) {
      return;
    }

    auto& player = stateManager->getState().player;
    const int partySize = static_cast<int>(player.party.size());
    if (partySize == 0) {
      return;
    }

    int nextIndex = player.currentPartyMemberIndex + directionDelta;
    if (nextIndex < 0) {
      nextIndex = partySize - 1;
    } else if (nextIndex >= partySize) {
      nextIndex = 0;
    }

    stateManager->enqueueAction(stateManager->getActionData(),
                                state::actions::setCurrentPartyMember(nextIndex),
                                0);
  }

} // namespace ui

export {

// --- from ui/components/TiledOverlay.h ---
#if defined(MIYOOA30) || defined(MIYOOMINI)

#else

#endif

namespace ui {

struct TiledOverlayProps {
  int width = 0;
  int height = 0;
  bmin::String spriteName = "ui_overlay_256";
  int alpha = 40;
};

// Tiles a sprite into a Quad-sized offscreen texture (edges clip to the texture),
// then blits it with alpha. Decorative — does not take input.
class TiledOverlay : public UiElement {
private:
  TiledOverlayProps props;
  SDL_Texture* renderTexture = nullptr;
  int currentWidth = 0;
  int currentHeight = 0;

  void createRenderTexture();
  void destroyRenderTexture();

public:
  TiledOverlay(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~TiledOverlay() override;

  void setProps(const TiledOverlayProps& _props);
  TiledOverlayProps& getProps();
  const TiledOverlayProps& getProps() const;

  bool checkMouseDownEvent(int mouseX,
                           int mouseY,
                           int button,
                           bmin::DynArray<UiElement*> additionalElements = {}) override;
  bool checkMouseUpEvent(int mouseX,
                         int mouseY,
                         int button,
                         bmin::DynArray<UiElement*> additionalElements = {}) override;
  bool checkHoverEvent(int mouseX,
                       int mouseY,
                       bmin::DynArray<UiElement*> additionalElements = {}) override;
  bool checkMouseWheelEvent(int mouseX,
                            int mouseY,
                            int delta,
                            bmin::DynArray<UiElement*> additionalElements = {}) override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

namespace ui {

TiledOverlay::TiledOverlay(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  shouldPropagateEventsToChildren = false;
}

TiledOverlay::~TiledOverlay() { destroyRenderTexture(); }

void TiledOverlay::setProps(const TiledOverlayProps& _props) {
  props = _props;
  build();
}

TiledOverlayProps& TiledOverlay::getProps() { return props; }

const TiledOverlayProps& TiledOverlay::getProps() const { return props; }

void TiledOverlay::createRenderTexture() {
  if (renderTexture != nullptr && currentWidth == style.width &&
      currentHeight == style.height) {
    return;
  }

  destroyRenderTexture();

  auto& draw = window->getDraw();
  auto* renderer = draw.getSdlRenderer();
  if (style.width <= 0 || style.height <= 0 || renderer == nullptr) {
    return;
  }

  renderTexture = SDL_CreateTexture(renderer,
                                    SDL_PIXELFORMAT_RGBA8888,
                                    SDL_TEXTUREACCESS_TARGET,
                                    style.width,
                                    style.height);
  if (renderTexture == nullptr) {
    LOG(ERROR) << "TiledOverlay::createRenderTexture - Failed to create texture: "
               << SDL_GetError() << LOG_ENDL;
    return;
  }

  SDL_SetTextureBlendMode(renderTexture, SDL_BLENDMODE_BLEND);
  currentWidth = style.width;
  currentHeight = style.height;
}

void TiledOverlay::destroyRenderTexture() {
  if (renderTexture != nullptr) {
    SDL_DestroyTexture(renderTexture);
    renderTexture = nullptr;
    currentWidth = 0;
    currentHeight = 0;
  }
}

bool TiledOverlay::checkMouseDownEvent(int, int, int, bmin::DynArray<UiElement*>) {
  return false;
}

bool TiledOverlay::checkMouseUpEvent(int, int, int, bmin::DynArray<UiElement*>) {
  return false;
}

bool TiledOverlay::checkHoverEvent(int, int, bmin::DynArray<UiElement*>) {
  return false;
}

bool TiledOverlay::checkMouseWheelEvent(int, int, int, bmin::DynArray<UiElement*>) {
  return false;
}

void TiledOverlay::build() {
  style.width = props.width;
  style.height = props.height;
  createRenderTexture();
}

void TiledOverlay::render(int dt) {
  if (renderTexture == nullptr || props.spriteName.empty()) {
    return;
  }

  auto& draw = window->getDraw();
  auto& store = window->getStore();
  auto* renderer = draw.getSdlRenderer();
  if (renderer == nullptr) {
    return;
  }

  auto& sprite = store.getSprite(bmin::toStringView(props.spriteName));
  const int spriteW = sprite.w;
  const int spriteH = sprite.h;
  if (spriteW <= 0 || spriteH <= 0) {
    return;
  }

  auto* previousTarget = SDL_GetRenderTarget(renderer);
  SDL_SetRenderTarget(renderer, renderTexture);
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
  SDL_RenderClear(renderer);

  // Tile into the Quad-sized texture. Overflowing edge tiles are clipped by the
  // render target, so full-sprite draws are safe here.
  for (int y = 0; y < style.height; y += spriteH) {
    for (int x = 0; x < style.width; x += spriteW) {
      draw.drawSprite(sprite,
                      sdl2w::RenderableParams{
                          .scale = {1.0, 1.0},
                          .x = x,
                          .y = y,
                          .centered = false,
                      });
    }
  }

  SDL_SetRenderTarget(renderer, previousTarget);

  const int scaledWidth = static_cast<int>(style.width * style.scale);
  const int scaledHeight = static_cast<int>(style.height * style.scale);
  SDL_SetTextureAlphaMod(renderTexture, static_cast<Uint8>(props.alpha));
  const SDL_Rect destRect = {style.x, style.y, scaledWidth, scaledHeight};
  SDL_RenderCopy(renderer, renderTexture, nullptr, &destRect);

  UiElement::render(dt);
}

} // namespace ui
