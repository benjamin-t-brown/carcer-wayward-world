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
  auto scaledWidth = static_cast<int>(style.width * style.scale);
  auto scaledHeight = static_cast<int>(style.height * style.scale);

  // Only recreate if size changed
  if (renderTexture == nullptr || currentWidth != scaledWidth ||
      currentHeight != scaledHeight) {
    destroyRenderTexture();

    auto& draw = window->getDraw();
    auto renderer = draw.getSdlRenderer();

    if (scaledWidth > 0 && scaledHeight > 0) {
      renderTexture = SDL_CreateTexture(renderer,
                                        SDL_PIXELFORMAT_RGBA8888,
                                        SDL_TEXTUREACCESS_TARGET,
                                        scaledWidth,
                                        scaledHeight);

      if (renderTexture) {
        SDL_SetTextureBlendMode(renderTexture, SDL_BLENDMODE_BLEND);
        currentWidth = scaledWidth;
        currentHeight = scaledHeight;
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

bool Quad::checkMouseDownEvent(int mouseX, int mouseY, int button) {
  // Check if click is within bounds using utility function
  if (isInBoundsScaled(mouseX, mouseY, this)) {
    isClicked = true;
    // Check children first (front to back)
    if (shouldPropagateEventsToChildren) {
      for (auto it = children.rbegin(); it != children.rend(); ++it) {
        if ((*it)->checkMouseDownEvent(mouseX - style.x, mouseY - style.y, button)) {
          return true;
        }
      }
    }

    for (auto& observer : eventObservers) {
      observer->onMouseDown(mouseX - style.x, mouseY - style.y, button);
    }

    return true;
  }

  return false;
}

bool Quad::checkMouseUpEvent(int mouseX, int mouseY, int button) {
  // Check if click is within bounds using utility function
  if (shouldPropagateEventsToChildren) {
    // Check children first (front to back)
    for (auto it = children.rbegin(); it != children.rend(); ++it) {
      (*it)->checkMouseUpEvent(mouseX - style.x, mouseY - style.y, button);
    }
  }

  if (isInBoundsScaled(mouseX, mouseY, this)) {
    if (isClicked) {
      // click event happens when mouse up occurs inside this element
      // after a mouse down also occurred inside this element.
      for (auto& observer : eventObservers) {
        observer->onClick(mouseX - style.x, mouseY - style.y, button);
      }
    }
  }

  for (auto& observer : eventObservers) {
    observer->onMouseUp(mouseX - style.x, mouseY - style.y, button);
  }
  isClicked = false;

  return true;
}

bool Quad::checkHoverEvent(int mouseX, int mouseY) {
  if (shouldPropagateEventsToChildren) {
    for (auto& child : children) {
      child->checkHoverEvent(mouseX - style.x, mouseY - style.y);
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

bool Quad::checkMouseWheelEvent(int mouseX, int mouseY, int delta) {
  if (isInBoundsScaled(mouseX, mouseY, this)) {
    if (shouldPropagateEventsToChildren) {
      for (auto& child : children) {
        child->checkMouseWheelEvent(mouseX - style.x, mouseY - style.y, delta);
      }
    }

    for (auto& observer : eventObservers) {
      observer->onMouseWheel(mouseX - style.x, mouseY - style.y, delta);
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

  // Calculate scaled dimensions
  auto scaledWidth = static_cast<int>(style.width * style.scale);
  auto scaledHeight = static_cast<int>(style.height * style.scale);

  // Save current render target
  auto previousTarget = SDL_GetRenderTarget(renderer);

  // Set render target to our texture
  SDL_SetRenderTarget(renderer, renderTexture);

  // Clear the texture with transparency
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
  SDL_RenderClear(renderer);

  draw.drawRect(0, 0, scaledWidth, scaledHeight, props.bgColor);

  // Render background sprite if specified
  if (!props.bgSprite.empty()) {
    auto& spriteData = store.getSprite(props.bgSprite);

    sdl2w::RenderableParamsEx params;
    params.x = 0;
    params.y = 0;
    params.w = scaledWidth;
    params.h = scaledHeight;
    params.scale = {1.0, 1.0};
    params.centered = false;

    draw.drawSprite(spriteData, params);
  }

  // Draw border as four rectangles (top, right, bottom, left)
  auto bs = props.borderSize;

  if (bs > 0) {
    // Top border
    draw.drawRect(0, 0, scaledWidth, bs, props.borderColor);
    // Bottom border
    draw.drawRect(0, scaledHeight - bs, scaledWidth, bs, props.borderColor);
    // Left border
    draw.drawRect(0, 0, bs, scaledHeight, props.borderColor);
    // Right border
    draw.drawRect(scaledWidth - bs, 0, bs, scaledHeight, props.borderColor);
  }

  // Render children (they render relative to 0,0 on the texture)
  UiElement::render(dt);

  // Restore previous render target
  SDL_SetRenderTarget(renderer, previousTarget);

  // Now render the texture at the actual position
  SDL_Rect destRect = {style.x, style.y, scaledWidth, scaledHeight};
  SDL_RenderCopy(renderer, renderTexture, nullptr, &destRect);
}

} // namespace ui
