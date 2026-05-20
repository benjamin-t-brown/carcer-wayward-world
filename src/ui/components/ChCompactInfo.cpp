#include "ChCompactInfo.h"
#include "lib/sdl2w/Defines.h"
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

const std::pair<int, int> ChCompactInfo::getDims() const {
  int w = props.spriteBoxSize + props.statusIconSize * props.numStatusColumns;
  int h = props.spriteBoxSize + fontHeight;
  return {static_cast<int>(w * style.scale), static_cast<int>(h * style.scale)};
}

void ChCompactInfo::build() {
  children.clear();

  style.height = props.spriteBoxSize + fontHeight;

  auto container = new Quad(window, this);
  auto& containerStyle = container->getStyle();
  containerStyle.x = style.x;
  containerStyle.y = style.y;
  containerStyle.width = style.width;
  containerStyle.height = style.height;
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

  const int textY = style.y + static_cast<int>(props.spriteBoxSize * style.scale);

  auto healthText = new TextLine(window, this);
  auto& healthStyle = healthText->getStyle();
  healthStyle.x = style.x;
  healthStyle.y = textY;
  healthStyle.fontFamily = FontFamily::H3;
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
  manaStyle.x = style.x + static_cast<int>(props.spriteBoxSize * style.scale);
  manaStyle.y = textY;
  manaStyle.fontFamily = FontFamily::H3;
  manaStyle.fontSize = style.fontSize;
  manaStyle.fontColor = Colors::Blue;
  manaStyle.textAlign = TextAlign::LEFT_TOP;
  TextLineProps manaProps;
  TextBlock manaBlock;
  manaBlock.text = std::to_string(props.mana);
  manaProps.textBlocks.push_back(manaBlock);
  manaText->setProps(manaProps);
  addChild(manaText);
}

void ChCompactInfo::render(int dt) { UiElement::render(dt); }

} // namespace ui
