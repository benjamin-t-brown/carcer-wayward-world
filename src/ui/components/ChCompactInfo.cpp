#include "ChCompactInfo.h"
#include "lib/sdl2w/Defines.h"
#include "ui/colors.h"
#include "ui/elements/OutsetRectangle.h"
#include "ui/elements/Quad.h"
#include "ui/elements/TextLine.h"

namespace ui {

ChCompactInfo::ChCompactInfo(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  style.fontSize = sdl2w::TEXT_SIZE_18;
}

void ChCompactInfo::setProps(const ChCompactInfoProps& _props) {
  props = _props;
  build();
}

ChCompactInfoProps& ChCompactInfo::getProps() { return props; }

const ChCompactInfoProps& ChCompactInfo::getProps() const { return props; }

int ChCompactInfo::getContentWidth() const {
  return props.spriteBoxSize + props.statusIconSize * props.numStatusColumns;
}

int ChCompactInfo::getContentHeight() const {
  return props.spriteBoxSize + fontHeight;
}

const std::pair<int, int> ChCompactInfo::getDims() const {
  const int contentW = getContentWidth();
  const int contentH = getContentHeight();
  const int w = contentW + props.padding * 2;
  const int h = contentH + props.padding * 2;
  return {static_cast<int>(w * style.scale), static_cast<int>(h * style.scale)};
}

void ChCompactInfo::build() {
  children.clear();

  const int contentW = getContentWidth();
  const int contentH = getContentHeight();
  const int scaledPadding = static_cast<int>(props.padding * style.scale);
  const int contentX = style.x + scaledPadding;
  const int contentY = style.y + scaledPadding;

  style.height = contentH + props.padding * 2;

  auto container = new Quad(window, this);
  auto& containerStyle = container->getStyle();
  containerStyle.x = contentX;
  containerStyle.y = contentY;
  containerStyle.width = contentW;
  containerStyle.height = contentH;
  containerStyle.scale = style.scale;
  container->setProps(QuadProps{});
  addChild(container);

  auto spriteRect = new OutsetRectangle(window, container);
  auto& spriteRectStyle = spriteRect->getStyle();
  spriteRectStyle.x = 0;
  spriteRectStyle.y = 0;
  spriteRectStyle.width = props.spriteBoxSize;
  spriteRectStyle.height = props.spriteBoxSize;
  spriteRect->setProps(OutsetRectangleProps{
      .color = props.spriteBgColor,
      .colorTopRight = props.spriteBorderColor1,
      .colorBottomLeft = props.spriteBorderColor2,
      .borderSize = props.spriteBorderSize,
  });
  container->addChild(spriteRect);

  auto sprite = new Quad(window, container);
  auto& spriteStyle = sprite->getStyle();
  spriteStyle.x = 1;
  spriteStyle.y = 1;
  spriteStyle.width = props.spriteBoxSize;
  spriteStyle.height = props.spriteBoxSize;
  QuadProps spriteProps;
  sprite->setProps(QuadProps{
      .bgSprite = props.characterSpriteName,
  });
  container->addChild(sprite);

  int gridX = props.spriteBoxSize;
  int gridY = 0;
  int seSpritesPerColumn = props.spriteBoxSize / props.statusIconSize;
  for (int j = 0; j < props.numStatusColumns; j++) {
    for (int i = 0; i < seSpritesPerColumn; i++) {
      const int index = i + j * seSpritesPerColumn;
      if (index >= props.statusEffectSpriteNames.size()) {
        break;
      }

      auto statusIcon = new Quad(window, container);
      auto& statusIconStyle = statusIcon->getStyle();
      statusIconStyle.x = gridX + props.statusIconSize * j;
      statusIconStyle.y = gridY + props.statusIconSize * i;
      statusIconStyle.width = props.statusIconSize;
      statusIconStyle.height = props.statusIconSize;
      statusIcon->setProps(QuadProps{
          .bgSprite = props.statusEffectSpriteNames[index],
      });
      container->addChild(statusIcon);
    }
  }

  const int textY = contentY + static_cast<int>(props.spriteBoxSize * style.scale) - 4;

  auto healthText = new TextLine(window, this);
  auto& healthStyle = healthText->getStyle();
  healthStyle.x = contentX;
  healthStyle.y = textY;
  setBaseFontConfig(healthStyle, BaseFontConfig::MODAL_TEXT_BOLD);
  healthStyle.fontSize = style.fontSize;
  healthStyle.fontColor = Colors::Red;
  healthStyle.textAlign = TextAlign::LEFT_TOP;
  TextLineProps healthProps;
  TextBlock healthBlock;
  healthBlock.text = std::to_string(props.hp);
  healthProps.textBlocks.push_back(healthBlock);
  healthText->setProps(healthProps);
  addChild(healthText);

  auto manaText = new TextLine(window, this);
  auto& manaStyle = manaText->getStyle();
  manaStyle.x = contentX + static_cast<int>(props.spriteBoxSize * style.scale);
  manaStyle.y = textY;
  setBaseFontConfig(manaStyle, BaseFontConfig::MODAL_TEXT_BOLD);
  manaStyle.fontSize = style.fontSize;
  manaStyle.fontColor = Colors::LightBlue;
  manaStyle.textAlign = TextAlign::LEFT_TOP;
  TextLineProps manaProps;
  TextBlock manaBlock;
  manaBlock.text = std::to_string(props.mana);
  manaProps.textBlocks.push_back(manaBlock);
  manaText->setProps(manaProps);
  addChild(manaText);
}

void ChCompactInfo::render(int dt) {
  if (props.isSelected) {
    auto& draw = window->getDraw();
    auto [scaledWidth, scaledHeight] = getDims();
    const float lineWidth = 1.f;
    const int x0 = style.x;
    const int y0 = style.y;
    const int x1 = style.x + scaledWidth - 1;
    const int y1 = style.y + scaledHeight - 1;
    const auto color = Colors::ButtonModalSelected;

    draw.drawLine({x0, y0}, {x1, y0}, lineWidth, color);
    draw.drawLine({x1, y0}, {x1, y1}, lineWidth, color);
    draw.drawLine({x0, y1}, {x1, y1}, lineWidth, color);
    draw.drawLine({x0, y0}, {x0, y1}, lineWidth, color);
  }
  UiElement::render(dt);
}

} // namespace ui
