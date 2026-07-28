#include "MapView.h"
#include "bmin/String.h"
#include "bmin/StringInterop.h"
#include "game/map/MapWalkability.h"
#include "game/map/TileFields.h"
#include "model/instances/CharacterPlayer.h"
#include "model/templates/CharacterTemplate.h"
#include "sdl2w/Animation.h"
#include "sdl2w/Draw.h"
#include "state/StateManager.h"
#include "ui/FontScale.h"
#include "ui/colors.h"
#include <exception>
#include <cmath>

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
  const auto& map = world.currentMap;
  if (map.width <= 0 || map.height <= 0) {
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

  auto spriteW = map.spriteWidth > 0 ? map.spriteWidth : 28;
  auto spriteH = map.spriteHeight > 0 ? map.spriteHeight : 32;
  if (spriteW <= 0 || spriteH <= 0 || style.scale <= 0.f) {
    return std::nullopt;
  }

  const auto mapPx =
      static_cast<int>((screenX - contentX) / style.scale) + world.camera.camX;
  const auto mapPy =
      static_cast<int>((screenY - contentY) / style.scale) + world.camera.camY;
  const auto tileX = mapPx / spriteW;
  const auto tileY = mapPy / spriteH;
  if (tileX < 0 || tileY < 0 || tileX >= map.width || tileY >= map.height) {
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

void MapView::renderDamageParticles(const model::World& world,
                                    const model::MapInstance& map,
                                    sdl2w::Draw& draw,
                                    sdl2w::Store& store,
                                    int contentX,
                                    int contentY,
                                    int spriteW,
                                    int spriteH,
                                    int fontScale) const {
  if (world.damageParticles.empty() || style.scale <= 0.f) {
    return;
  }

  for (size_t i = 0; i < world.damageParticles.size(); i++) {
    const auto& particle = world.damageParticles[i];
    if (!game::isTileCurrentlyVisible(map, particle.tileX, particle.tileY)) {
      continue;
    }
    if (!store.anims.contains(particle.animationName)) {
      continue;
    }

    auto screenX =
        contentX + static_cast<int>((particle.tileX * spriteW - world.camera.camX) * style.scale);
    auto screenY =
        contentY + static_cast<int>((particle.tileY * spriteH - world.camera.camY) * style.scale);

    auto centerX = screenX + static_cast<int>(spriteW * style.scale / 2);
    auto centerY = screenY + static_cast<int>(spriteH * style.scale / 2);

    auto animation = store.createAnimation(bmin::toStringView(particle.animationName));
    if (!animation.isInitialized()) {
      continue;
    }
    animation.start();
    animation.update(particle.lifetime.t);

    draw.drawAnimation(
        animation,
        sdl2w::RenderableParamsEx{
            .scale = {style.scale, style.scale},
            .x = centerX,
            .y = centerY,
            .centered = true,
        });

    if (particle.value != 0) {
      auto damageText = bmin::toString(abs(particle.value));
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

void MapView::render(int /*dt*/) {
  auto* stateManager = getStateManager();
  if (!stateManager) {
    return;
  }

  const auto& state = stateManager->getState();
  const auto& world = state.world;
  const auto& map = world.currentMap;
  if (map.width <= 0 || map.height <= 0) {
    return;
  }

  auto contentX = style.x;
  auto contentY = style.y;
  auto contentW = static_cast<int>(style.width * style.scale);
  auto contentH = static_cast<int>(style.height * style.scale);
  if (contentW <= 0 || contentH <= 0) {
    return;
  }

  auto spriteW = map.spriteWidth > 0 ? map.spriteWidth : 28;
  auto spriteH = map.spriteHeight > 0 ? map.spriteHeight : 32;
  auto scaledSpriteW = static_cast<int>(spriteW * style.scale);
  auto scaledSpriteH = static_cast<int>(spriteH * style.scale);
  if (scaledSpriteW <= 0 || scaledSpriteH <= 0) {
    return;
  }

  auto& draw = window->getDraw();
  auto& store = window->getStore();

  // Draw whole sprites; overdraw past the content rect is fine (chrome draws on top).
  auto drawMapSprite = [&](sdl2w::Sprite& sprite, int screenX, int screenY, bool flipped = false) {
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

  // One sprite per cell: highest non-empty tile at or below tileLayerNumber.
  // Never-seen cells draw black; explored-but-not-visible get fog overlay.
  for (auto y = 0; y < map.height; y++) {
    for (auto x = 0; x < map.width; x++) {
      auto screenX =
          contentX + static_cast<int>((x * spriteW - world.camera.camX) * style.scale);
      auto screenY =
          contentY + static_cast<int>((y * spriteH - world.camera.camY) * style.scale);

      const auto* tile = game::resolveTileToRender(map, x, y);
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
        if (const auto* surfaceTile = game::tileAtCurrentLayer(map, x, y)) {
          for (size_t fi = 0; fi < surfaceTile->fields.size(); fi++) {
            const auto& field = surfaceTile->fields[fi];
            const auto fieldSpriteName = game::tileFieldSpriteName(field);
            if (!store.sprites.contains(fieldSpriteName)) {
              continue;
            }
            auto& fieldSprite = store.getSprite(bmin::toStringView(fieldSpriteName));
            drawMapSprite(fieldSprite, screenX, screenY);
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

  auto* database = getDatabase();
  for (size_t ii = 0; ii < map.items.size(); ii++) {
    const auto& item = map.items[ii];
    if (!database) {
      break;
    }
    if (!game::isTileCurrentlyVisible(map, item.x, item.y)) {
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

    auto& sprite = store.getSprite(bmin::toStringView(spriteName));
    drawMapSprite(sprite, screenX, screenY);
  }

  const auto& party = state.player.party;
  const bmin::String* activeCharacterId = nullptr;
  if (world.combat.active && !world.combat.activeCharacterId.empty()) {
    activeCharacterId = &world.combat.activeCharacterId;
  }

  auto drawCharacter = [&](const model::CharacterInstance& character) {
    if (!game::isTileCurrentlyVisible(map, character.x, character.y)) {
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
      spriteName =
          model::characterPlayerGetSpriteAtIndexOffset(*member, character.spriteIndexOffset);
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
        contentX + static_cast<int>((character.x * spriteW - world.camera.camX) * style.scale);
    auto screenY =
        contentY + static_cast<int>((character.y * spriteH - world.camera.camY) * style.scale);

    auto& sprite = store.getSprite(bmin::toStringView(spriteName));
    drawMapSprite(sprite, screenX, screenY, model::isCharacterFacingLeft(character));
  };

  const model::CharacterInstance* activeCharacter = nullptr;
  for (size_t ci = 0; ci < map.characters.size(); ci++) {
    const auto& character = map.characters[ci];
    if (activeCharacterId != nullptr && character.id == *activeCharacterId) {
      activeCharacter = &character;
      continue;
    }
    drawCharacter(character);
  }
  if (activeCharacter != nullptr) {
    drawCharacter(*activeCharacter);
  }

  renderDamageParticles(world,
                        map,
                        draw,
                        store,
                        contentX,
                        contentY,
                        spriteW,
                        spriteH,
                        state.settings.fontScale);

  if (world.actionMode != model::WorldActionMode::NONE && world.actionAimTile) {
    const auto aimX = world.actionAimTile->x;
    const auto aimY = world.actionAimTile->y;
    const auto screenX =
        contentX + static_cast<int>((aimX * spriteW - world.camera.camX) * style.scale);
    const auto screenY =
        contentY + static_cast<int>((aimY * spriteH - world.camera.camY) * style.scale);

    if (screenX + scaledSpriteW > contentX && screenX < contentX + contentW &&
        screenY + scaledSpriteH > contentY && screenY < contentY + contentH) {
      draw.drawRect(screenX, screenY, scaledSpriteW, scaledSpriteH, actionAimFillColor);
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

} // namespace ui
