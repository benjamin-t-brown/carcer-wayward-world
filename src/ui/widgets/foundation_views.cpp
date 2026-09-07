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

module carcer.ui.widgets.foundation;
#include "macros.h"

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
