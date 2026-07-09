#include "ChCompactInfo.h"
#include "ui/colors.h"
#include "ui/elements/OutsetRectangle.h"
#include "ui/elements/Quad.h"
#include "ui/elements/TextLine.h"

namespace ui {

ChCompactInfo::ChCompactInfo(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {}

void ChCompactInfo::setProps(const ChCompactInfoProps& _props) {
  props = _props;
  build();
}

ChCompactInfoProps& ChCompactInfo::getProps() { return props; }

const ChCompactInfoProps& ChCompactInfo::getProps() const { return props; }

int ChCompactInfo::getContentWidth() const {
  return props.spriteBoxSize + props.statusIconSize * props.numStatusColumns;
}

int ChCompactInfo::getContentHeight() const { return props.spriteBoxSize + fontHeight; }

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
  container->setPos(contentX, contentY);
  container->setScale(style.scale);
  container->setProps(QuadProps{
      .width = contentW,
      .height = contentH,
  });
  addChild(container);

  auto spriteRect = new OutsetRectangle(window, container);
  spriteRect->setPos(0, 0);
  spriteRect->setProps(OutsetRectangleProps{
      .width = props.spriteBoxSize,
      .height = props.spriteBoxSize,
      .color = props.spriteBgColor,
      .colorTopRight = props.spriteBorderColor1,
      .colorBottomLeft = props.spriteBorderColor2,
      .borderSize = props.spriteBorderSize,
  });
  container->addChild(spriteRect);

  auto sprite = new Quad(window, container);
  sprite->setPos(1, 1);
  sprite->setProps(QuadProps{
      .width = props.spriteBoxSize,
      .height = props.spriteBoxSize,
      .bgSprite = props.characterSpriteName,
  });
  container->addChild(sprite);

  int gridX = props.spriteBoxSize;
  int gridY = 0;
  int seSpritesPerColumn = props.spriteBoxSize / props.statusIconSize;
  for (int j = 0; j < props.numStatusColumns; j++) {
    for (int i = 0; i < seSpritesPerColumn; i++) {
      const int index = i + j * seSpritesPerColumn;
      if (static_cast<size_t>(index) >= props.statusEffectSpriteNames.size()) {
        break;
      }

      auto statusIcon = new Quad(window, container);
      statusIcon->setPos(gridX + props.statusIconSize * j,
                         gridY + props.statusIconSize * i);
      statusIcon->setProps(QuadProps{
          .width = props.statusIconSize,
          .height = props.statusIconSize,
          .bgSprite = props.statusEffectSpriteNames[index],
      });
      container->addChild(statusIcon);
    }
  }

  const int textY = contentY + static_cast<int>(props.spriteBoxSize * style.scale) - 4;

  TextFontProps font;
  setBaseFontConfig(font, BaseFontConfig::MODAL_TEXT_BOLD);

  auto healthText = new TextLine(window, this);
  healthText->setPos(contentX, textY);
  healthText->setScale(1.f);
  TextLineProps healthProps;
  healthProps.fontFamily = font.fontFamily;
  healthProps.fontSize = props.fontSize;
  healthProps.fontColor = Colors::Red;
  healthProps.textAlign = TextAlign::LEFT_TOP;
  TextBlock healthBlock;
  healthBlock.text = bmin::toString(props.hp);
  healthProps.textBlocks.pushBack(healthBlock);
  healthText->setProps(healthProps);
  addChild(healthText);

  auto manaText = new TextLine(window, this);
  manaText->setPos(contentX + static_cast<int>(props.spriteBoxSize * style.scale), textY);
  manaText->setScale(1.f);
  TextLineProps manaProps;
  manaProps.fontFamily = font.fontFamily;
  manaProps.fontSize = props.fontSize;
  manaProps.fontColor = Colors::LightBlue;
  manaProps.textAlign = TextAlign::LEFT_TOP;
  TextBlock manaBlock;
  manaBlock.text = bmin::toString(props.mana);
  manaProps.textBlocks.pushBack(manaBlock);
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
