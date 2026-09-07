module;
#include <cstddef>
#include <cstdint>
#include <utility>
#include <SDL.h>
#include <SDL_pixels.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_pixels.h>
#include <stdexcept>
#include <optional>
#include <algorithm>

module carcer.ui.widgets.foundation;
#include "macros.h"

namespace ui {

HorizontalList::HorizontalList(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  build();
}

void HorizontalList::setProps(const HorizontalListProps& _props) {
  props = _props;
  build();
}

HorizontalListProps& HorizontalList::getProps() { return props; }

const HorizontalListProps& HorizontalList::getProps() const { return props; }

void HorizontalList::setSelectedIndex(int index) {
  if (index >= -1 && index < static_cast<int>(children.size())) {
    selectedIndex = index;
  }
}

int HorizontalList::getSelectedIndex() const { return selectedIndex; }

void HorizontalList::clearSelection() { selectedIndex = -1; }

void HorizontalList::addListItem(UiElement* item) { addChild(item); }

void HorizontalList::addListItems(const bmin::DynArray<UiElement*>& items) {
  for (auto* item : items) {
    addListItem(item);
  }
}

void HorizontalList::removeListItemAtIndex(size_t index) {
  if (index < children.size()) {
    children.erase(children.begin() + index);
  }
}

const std::pair<int, int> HorizontalList::getDims() const {
  auto w = (props.lineWidth + props.lineGap) * static_cast<int>(children.size());
  auto h = props.height > 0 ? props.height : style.height;
  return {static_cast<int>(w * style.scale), static_cast<int>(h * style.scale)};
}

void HorizontalList::build() {
  if (props.height > 0) {
    style.height = props.height;
  }
  for (size_t i = 0; i < children.size(); i++) {
    auto& child = children[i];
    child->setPos(style.x + (props.lineWidth + props.lineGap) * static_cast<int>(i),
                  style.y);
    child->build();
  }
}

void HorizontalList::render(int dt) { UiElement::render(dt); }

} // namespace ui


namespace ui {

OutsetRectangle::OutsetRectangle(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  // Default props
}

void OutsetRectangle::setProps(const OutsetRectangleProps& _props) {
  props = _props;
  build();
}

OutsetRectangleProps& OutsetRectangle::getProps() { return props; }

const OutsetRectangleProps& OutsetRectangle::getProps() const { return props; }

const std::pair<int, int> OutsetRectangle::getDims() const {
  return {style.width, style.height};
}

void OutsetRectangle::build() {
  style.width = props.width;
  style.height = props.height;
}

void OutsetRectangle::render(int dt) {
  auto& draw = window->getDraw();

  // Calculate scaled dimensions
  int scaledX = static_cast<int>(style.x);
  int scaledY = static_cast<int>(style.y);
  int scaledWidth = static_cast<int>(style.width * style.scale);
  int scaledHeight = static_cast<int>(style.height * style.scale);
  int borderSize = static_cast<int>(props.borderSize * style.scale);

  // Draw the main rectangle
  draw.drawRect(scaledX + borderSize,
                scaledY + borderSize,
                scaledWidth - borderSize * 2,
                scaledHeight - borderSize * 2,
                props.color);

  // Draw outset border effect
  if (props.borderSize > 0) {
    // Top and Right borders
    draw.drawRect(scaledX, scaledY, scaledWidth, borderSize, props.colorTopRight);
    draw.drawRect(scaledX + scaledWidth - borderSize,
                  scaledY,
                  borderSize,
                  scaledHeight,
                  props.colorTopRight);
    // Bottom and Left borders
    draw.drawRect(scaledX,
                  scaledY + scaledHeight - borderSize,
                  scaledWidth,
                  borderSize,
                  props.colorBottomLeft);
    draw.drawRect(scaledX, scaledY, borderSize, scaledHeight, props.colorBottomLeft);

    if (borderSize > 1) {
      // Diagonal corners top left bottom right
      for (int i = 0; i < borderSize; i++) {
        auto topLeftX = scaledX;
        auto topLeftY = scaledY;
        draw.drawLine({topLeftX + i, topLeftY},
                      {topLeftX + i, topLeftY + i},
                      1,
                      props.colorTopRight);

        auto bottomRightX = scaledX + scaledWidth;
        auto bottomRightY = scaledY + scaledHeight;
        draw.drawLine({bottomRightX - borderSize + i, bottomRightY - borderSize},
                      {bottomRightX - borderSize + i, bottomRightY - borderSize + i},
                      1,
                      props.colorTopRight);
      }
    }
  }

  // Render children
  UiElement::render(dt);
}

} // namespace ui


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
                               bmin::DynArray<UiElement*> additionalElements) {
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
                             bmin::DynArray<UiElement*> additionalElements) {
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
                           bmin::DynArray<UiElement*> additionalElements) {
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
                                bmin::DynArray<UiElement*> additionalElements) {
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
  style.width = props.width;
  style.height = props.height;
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


namespace ui {

SpriteElement::SpriteElement(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {}

void SpriteElement::setProps(const SpriteElementProps& _props) {
  props = _props;
  build();
}

SpriteElementProps& SpriteElement::getProps() { return props; }

const SpriteElementProps& SpriteElement::getProps() const { return props; }

void SpriteElement::setSprite(const bmin::String& name) {
  props.spriteName = name;
  build();
}

const sdl2w::Sprite& SpriteElement::getSprite() const { return sprite; }

void SpriteElement::build() {
  style.width = props.width;
  style.height = props.height;
  if (props.spriteName.empty()) {
    sprite = sdl2w::Sprite{};
    return;
  }
  try {
    sprite = window->getStore().getSprite(bmin::toStringView(props.spriteName));
  } catch (const std::runtime_error& e) {
    LOG_LINE(ERROR)
        << (bmin::String("[ui] ERROR When setting Sprite for ui sprite element. Cannot get Sprite '") +
            props.spriteName.cStr() + "' because it has not been loaded.")
               .cStr()
        << LOG_ENDL;
    throw std::runtime_error(
        (bmin::String("Failed to get Sprite '") + props.spriteName.cStr() + "'").cStr());
  }
}

void SpriteElement::render(int dt) {
  auto& draw = window->getDraw();

  if (sprite.name.empty()) {
    return;
  }

  sdl2w::RenderableParamsEx params;
  params.x = style.x;
  params.y = style.y;
  params.w = style.width;
  params.h = style.height;
  params.scale = {style.scale, style.scale};
  params.centered = false;

  draw.drawSprite(sprite, params);
}

} // namespace ui


namespace ui {

TextLine::TextLine(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {}

bmin::String TextLine::getFontNameFromFamily(FontFamily fontFamily) {
  auto fontName = bmin::String("default");
  switch (fontFamily) {
  case FontFamily::TEXT:
    fontName = "text";
    break;
  case FontFamily::TEXT_BOLD:
    fontName = "text-bold";
    break;
  case FontFamily::DEFAULT:
    fontName = "default";
    break;
  case FontFamily::TITLE:
    fontName = "title";
    break;
  }
  return fontName;
}

sdl2w::RenderTextParams TextLine::makeRenderTextParams(const TextBlock& block) const {
  auto fontFamily = block.fontFamily.value_or(props.fontFamily);
  auto baseFontSize = block.fontSize.value_or(props.fontSize);
  auto fontColor = block.fontColor.value_or(props.fontColor);
  int fontScale = 0;
  try {
    auto stateManager = getStateManager();
    if (!stateManager) {
      throw std::runtime_error("StateManager not set");
    }
    fontScale = getStateManager()->getState().settings.fontScale;
  } catch (...) {
    // Some isolated UI tests do not initialize a StateManager.
    fontScale = 0;
  }

  sdl2w::RenderTextParams params;
  params.fontName = getFontNameFromFamily(fontFamily);
  params.fontSize = ui::applyFontScale(baseFontSize, fontScale);
  params.color = fontColor;
  params.centered = props.textAlign == TextAlign::CENTER;
  return params;
}

void TextLine::setProps(const TextLineProps& _props) {
  props = _props;
  build();
}

TextLineProps& TextLine::getProps() { return props; }

const TextLineProps& TextLine::getProps() const { return props; }

void TextLine::setPos(int x, int y) {
  UiElement::setPos(x, y);
  build();
}

void TextLine::setScale(float scale) {
  UiElement::setScale(scale);
  build();
}

std::pair<int, int> TextLine::calculateTextDims() const {
  if (props.textBlocks.empty()) {
    return {0, 0};
  }

  auto& draw = window->getDraw();
  int totalWidth = 0;
  int totalHeight = 0;

  for (const auto& block : props.textBlocks) {
    const bmin::String& measureStr = block.text.empty() ? bmin::String(" ") : block.text;
    auto [textWidth, textHeight] =
        draw.measureText(bmin::toStringView(measureStr), makeRenderTextParams(block));
    if (!block.text.empty()) {
      totalWidth += textWidth;
    }
    totalHeight = std::max(totalHeight, textHeight);
  }

  return {totalWidth, totalHeight};
}

const std::pair<int, int> TextLine::getDims() const {
  auto [width, height] = calculateTextDims();
  return {static_cast<int>(width * style.scale), static_cast<int>(height * style.scale)};
}

void TextLine::build() {
  textRenderables.clear();

  auto [totalWidth, totalHeight] = calculateTextDims();
  style.width = totalWidth;
  style.height = totalHeight;

  auto currentX = static_cast<int>(style.x * style.scale);
  auto currentY = static_cast<int>(style.y * style.scale);

  for (const auto& block : props.textBlocks) {
    if (block.text.empty()) {
      // Blank line: reserve vertical space via calculateTextDims(), nothing to draw.
      continue;
    }

    auto renderTextParams = makeRenderTextParams(block);
    auto [textWidth, textHeight] =
        window->getDraw().measureText(bmin::toStringView(block.text), renderTextParams);
    textHeight += 2; // HACK: Measure text doesn't seem accurate per height, so this will
                     // need overrides...

    auto tlParams = bmin::makeUnique<TextLineRenderTextParams>();
    tlParams->text = block.text;
    renderTextParams.x = currentX;
    renderTextParams.y = currentY;
    if (props.textAlign == TextAlign::LEFT_CENTER) {
      renderTextParams.y -= static_cast<int>((textHeight / 2.0) * style.scale);
    } else if (props.textAlign == TextAlign::LEFT_BOTTOM) {
      renderTextParams.y -= static_cast<int>(textHeight * style.scale);
    }
    tlParams->params = renderTextParams;
    textRenderables.pushBack(std::move(tlParams));

    currentX += static_cast<int>(textWidth * style.scale);
  }
}

void TextLine::render(int dt) {
  if (props.textBlocks.empty()) {
    return;
  }

  auto& draw = window->getDraw();

  for (const auto& rtParams : textRenderables) {
    draw.drawText(bmin::toStringView(rtParams->text), rtParams->params);
  }
}

} // namespace ui


namespace ui {

VerticalList::VerticalList(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  build();
}

void VerticalList::setProps(const VerticalListProps& _props) {
  props = _props;
  build();
}

VerticalListProps& VerticalList::getProps() { return props; }

const VerticalListProps& VerticalList::getProps() const { return props; }

void VerticalList::setSelectedIndex(int index) {
  if (index >= -1 && index < static_cast<int>(children.size())) {
    selectedIndex = index;
  }
}

int VerticalList::getSelectedIndex() const { return selectedIndex; }

void VerticalList::clearSelection() { selectedIndex = -1; }

void VerticalList::addListItem(UiElement* item) { addChild(item); }

void VerticalList::addListItems(const bmin::DynArray<UiElement*>& items) {
  for (int i = 0; i < static_cast<int>(items.size()); i++) {
    addListItem(items[i]);
  }
}

void VerticalList::removeListItemAtIndex(size_t index) {
  if (index < children.size()) {
    children.erase(children.begin() + index);
  }
}

const std::pair<int, int> VerticalList::getDims() const {
  auto w = props.width > 0 ? props.width : style.width;
  auto h = (props.lineHeight + props.lineGap) * static_cast<int>(children.size());
  return {static_cast<int>(w * style.scale), static_cast<int>(h * style.scale)};
}

void VerticalList::build() {
  if (props.width > 0) {
    style.width = props.width;
  }
  for (size_t i = 0; i < children.size(); i++) {
    auto& child = children[i];
    child->setPos(style.x, style.y + (props.lineHeight + props.lineGap) * static_cast<int>(i));
    child->build();
  }
}

void VerticalList::render(int dt) { UiElement::render(dt); }

} // namespace ui
