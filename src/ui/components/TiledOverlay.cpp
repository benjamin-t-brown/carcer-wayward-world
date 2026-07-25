#include "TiledOverlay.h"
#include "bmin/StringInterop.h"
#include "sdl2w/Draw.h"
#include "sdl2w/Logger.h"

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
