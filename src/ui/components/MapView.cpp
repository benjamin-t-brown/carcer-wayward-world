#include "MapView.h"
#include "bmin/String.h"
#include "bmin/StringInterop.h"
#include "model/MapWalkability.h"
#include "model/instances/CharacterPlayer.h"
#include "model/templates/CharacterTemplate.h"
#include "sdl2w/Draw.h"
#include "state/StateManager.h"
#include <exception>

namespace ui {

MapView::MapView(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {}

bool MapView::isCellCurrentlyVisible(const model::MapInstance& map, int x, int y) {
  const auto* tile = model::resolveTileToRender(map, x, y);
  return tile != nullptr && tile->isVisible;
}

void MapView::setProps(const MapViewProps& _props) {
  props = _props;
  build();
}

MapViewProps& MapView::getProps() { return props; }

const MapViewProps& MapView::getProps() const { return props; }

void MapView::build() {
  if (props.width > 0) {
    style.width = props.width;
  }
  if (props.height > 0) {
    style.height = props.height;
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
  auto drawMapSprite = [&](sdl2w::Sprite& sprite, int screenX, int screenY) {
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
                    });
  };

  // One sprite per cell: highest non-empty tile at or below tileLayerNumber.
  // Never-seen cells draw black; explored-but-not-visible get fog overlay.
  for (auto y = 0; y < map.height; y++) {
    for (auto x = 0; x < map.width; x++) {
      auto screenX =
          contentX + static_cast<int>((x * spriteW - world.camX) * style.scale);
      auto screenY =
          contentY + static_cast<int>((y * spriteH - world.camY) * style.scale);

      const auto* tile = model::resolveTileToRender(map, x, y);
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
    if (!isCellCurrentlyVisible(map, item.x, item.y)) {
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
        contentX + static_cast<int>((item.x * spriteW - world.camX) * style.scale);
    auto screenY =
        contentY + static_cast<int>((item.y * spriteH - world.camY) * style.scale);

    auto& sprite = store.getSprite(bmin::toStringView(spriteName));
    drawMapSprite(sprite, screenX, screenY);
  }

  const auto& party = state.player.party;
  for (size_t ci = 0; ci < map.characters.size(); ci++) {
    const auto& character = map.characters[ci];
    if (!isCellCurrentlyVisible(map, character.x, character.y)) {
      continue;
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
      spriteName = model::characterPlayerGetSprite(*member);
    } else if (database) {
      try {
        const auto& characterTemplate =
            database->getCharacterTemplate(bmin::toStringView(character.templateName));
        spriteName = model::characterGetSprite(characterTemplate);
      } catch (const std::exception&) {
        continue;
      }
    } else {
      continue;
    }

    if (spriteName.empty() || !store.sprites.contains(spriteName)) {
      continue;
    }

    auto screenX =
        contentX + static_cast<int>((character.x * spriteW - world.camX) * style.scale);
    auto screenY =
        contentY + static_cast<int>((character.y * spriteH - world.camY) * style.scale);

    auto& sprite = store.getSprite(bmin::toStringView(spriteName));
    drawMapSprite(sprite, screenX, screenY);
  }
}

} // namespace ui
