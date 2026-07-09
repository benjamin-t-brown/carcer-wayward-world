#include "Quad.h"
#include "lib/sdl2w/Draw.h"
#include "lib/sdl2w/Logger.h"
#include "ui/uiUtils.h"

namespace ui {

Quad::Quad(sdl2w::Window* _window, UiElement* _parent) : UiElement(_window, _parent) {
  build();
}

Quad::~Quad() { destroyRenderTexture(); }

void Quad::setProps(const QuadProps& _props) {
  props = _props;
  build();
}

QuadProps& Quad::getProps() { return props; }

const QuadProps& Quad::getProps() const { return props; }

void Quad::updatePosition(int x, int y) {
  style.x = x;
  style.y = y;
}

void Quad::createRenderTexture() {
  // Render target stays at logical size; scale is applied when blitting to screen.
  if (renderTexture == nullptr || currentWidth != style.width ||
      currentHeight != style.height) {
    destroyRenderTexture();

    auto& draw = window->getDraw();
    auto renderer = draw.getSdlRenderer();

    if (style.width > 0 && style.height > 0) {
      renderTexture = SDL_CreateTexture(renderer,
                                        SDL_PIXELFORMAT_RGBA8888,
                                        SDL_TEXTUREACCESS_TARGET,
                                        style.width,
                                        style.height);

      if (renderTexture) {
        SDL_SetTextureBlendMode(renderTexture, SDL_BLENDMODE_BLEND);
        currentWidth = style.width;
        currentHeight = style.height;
      } else {
        LOG(ERROR) << "Quad::createRenderTexture - Failed to create texture: "
                   << SDL_GetError() << LOG_ENDL;
      }
    }
  }
}

void Quad::destroyRenderTexture() {
  if (renderTexture != nullptr) {
    SDL_DestroyTexture(renderTexture);
    renderTexture = nullptr;
    currentWidth = 0;
    currentHeight = 0;
  }
}

namespace {

// Map screen-space coords within the quad to texture-local (logical) coords.
std::pair<int, int> toTextureCoords(int mouseX, int mouseY, const BaseStyle& style) {
  const int localX = static_cast<int>((mouseX - style.x) / style.scale);
  const int localY = static_cast<int>((mouseY - style.y) / style.scale);
  return {localX, localY};
}

} // namespace

bool Quad::checkMouseDownEvent(int mouseX,
                               int mouseY,
                               int button,
                               DynArray<UiElement*> additionalElements) {
  // Check if click is within bounds using utility function
  if (isInBoundsScaled(mouseX, mouseY, this)) {
    isClicked = true;
    auto [localX, localY] = toTextureCoords(mouseX, mouseY, style);
    // Check children first (front to back)
    if (shouldPropagateEventsToChildren) {
      for (auto it = children.rbegin(); it != children.rend(); ++it) {
        if ((*it)->checkMouseDownEvent(localX, localY, button)) {
          return true;
        }
      }
    }

    for (auto& observer : eventObservers) {
      observer->onMouseDown(localX, localY, button);
    }

    return true;
  }

  return false;
}

bool Quad::checkMouseUpEvent(int mouseX,
                             int mouseY,
                             int button,
                             DynArray<UiElement*> additionalElements) {
  auto [localX, localY] = toTextureCoords(mouseX, mouseY, style);
  if (shouldPropagateEventsToChildren) {
    // Check children first (front to back)
    for (auto it = children.rbegin(); it != children.rend(); ++it) {
      (*it)->checkMouseUpEvent(localX, localY, button);
    }
  }

  if (isInBoundsScaled(mouseX, mouseY, this)) {
    if (isClicked) {
      // click event happens when mouse up occurs inside this element
      // after a mouse down also occurred inside this element.
      for (auto& observer : eventObservers) {
        observer->onClick(localX, localY, button);
      }
    }
  }

  for (auto& observer : eventObservers) {
    observer->onMouseUp(localX, localY, button);
  }
  isClicked = false;

  return true;
}

bool Quad::checkHoverEvent(int mouseX,
                           int mouseY,
                           DynArray<UiElement*> additionalElements) {
  auto [localX, localY] = toTextureCoords(mouseX, mouseY, style);
  if (shouldPropagateEventsToChildren) {
    for (auto& child : children) {
      child->checkHoverEvent(localX, localY);
    }
  }

  if (isInBoundsScaled(mouseX, mouseY, this)) {
    isHovered = true;

    return true;
  } else {
    isHovered = false;
  }

  return false;
}

bool Quad::checkMouseWheelEvent(int mouseX,
                                int mouseY,
                                int delta,
                                DynArray<UiElement*> additionalElements) {
  if (isInBoundsScaled(mouseX, mouseY, this)) {
    auto [localX, localY] = toTextureCoords(mouseX, mouseY, style);
    if (shouldPropagateEventsToChildren) {
      for (auto& child : children) {
        child->checkMouseWheelEvent(localX, localY, delta);
      }
    }

    for (auto& observer : eventObservers) {
      observer->onMouseWheel(localX, localY, delta);
    }

    return true;
  }
  return false;
}

void Quad::build() {
  // Recreate render texture when size changes
  createRenderTexture();
}

void Quad::render(int dt) {
  if (renderTexture == nullptr) {
    return;
  }

  auto& draw = window->getDraw();
  auto& store = window->getStore();
  auto renderer = draw.getSdlRenderer();

  const int textureWidth = style.width;
  const int textureHeight = style.height;
  const int scaledWidth = static_cast<int>(style.width * style.scale);
  const int scaledHeight = static_cast<int>(style.height * style.scale);

  // Save current render target
  auto previousTarget = SDL_GetRenderTarget(renderer);

  // Set render target to our texture
  SDL_SetRenderTarget(renderer, renderTexture);

  // Clear the texture with transparency
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
  SDL_RenderClear(renderer);

  draw.drawRect(0, 0, textureWidth, textureHeight, props.bgColor);

  // Render background sprite if specified
  if (!props.bgSprite.empty()) {
    auto& spriteData = store.getSprite(bmin::toStringView(props.bgSprite));

    sdl2w::RenderableParamsEx params;
    params.x = 0;
    params.y = 0;
    params.w = textureWidth;
    params.h = textureHeight;
    params.scale = {1.0, 1.0};
    params.centered = false;

    draw.drawSprite(spriteData, params);
  }

  // Draw border as four rectangles (top, right, bottom, left)
  auto bs = props.borderSize;

  if (bs > 0) {
    // Top border
    draw.drawRect(0, 0, textureWidth, bs, props.borderColor);
    // Bottom border
    draw.drawRect(0, textureHeight - bs, textureWidth, bs, props.borderColor);
    // Left border
    draw.drawRect(0, 0, bs, textureHeight, props.borderColor);
    // Right border
    draw.drawRect(textureWidth - bs, 0, bs, textureHeight, props.borderColor);
  }

  // Render children (they render relative to 0,0 on the texture)
  UiElement::render(dt);

  // Restore previous render target
  SDL_SetRenderTarget(renderer, previousTarget);

  // Blit texture scaled to screen position
  SDL_Rect destRect = {style.x, style.y, scaledWidth, scaledHeight};
  SDL_RenderCopy(renderer, renderTexture, nullptr, &destRect);
}

} // namespace ui
