#include "GameChStatus.h"
#include "ui/elements/Quad.h"
#include "ui/elements/TextLine.h"
#include <algorithm>

namespace ui {

GameChStatus::GameChStatus(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {}

void GameChStatus::setProps(const GameChStatusProps& _props) {
  props = _props;
  build();
}

GameChStatusProps& GameChStatus::getProps() { return props; }

const GameChStatusProps& GameChStatus::getProps() const { return props; }

const std::pair<int, int> GameChStatus::getDims() const {
  return {style.width, style.height};
}

void GameChStatus::build() {
  children.clear();

  auto container = std::make_unique<Quad>(window, this);
  BaseStyle containerStyle;
  containerStyle.x = style.x;
  containerStyle.y = style.y;
  containerStyle.width = style.width;
  containerStyle.height = style.height;
  containerStyle.scale = 1.0f;
  container->setStyle(containerStyle);

  QuadProps containerProps;
  containerProps.bgColor = props.backgroundColor;
  containerProps.borderColor = props.borderColor;
  containerProps.borderSize = props.borderSize;
  container->setProps(containerProps);

  const int topRowY = props.padding;
  const int spriteSize = std::max(0, props.spriteSize);
  const int statusIconSize = std::max(0, props.statusIconSize);
  const int statusIconGap = std::max(0, props.statusIconGap);
  const int statusGridCols = std::max(1, props.statusGridCols);

  auto sprite = std::make_unique<Quad>(window, container.get());
  BaseStyle spriteStyle;
  spriteStyle.x = props.padding;
  spriteStyle.y = topRowY;
  spriteStyle.width = spriteSize;
  spriteStyle.height = spriteSize;
  spriteStyle.scale = 1.0f;
  sprite->setStyle(spriteStyle);

  QuadProps spriteProps;
  spriteProps.bgColor = Colors::DarkBlue;
  spriteProps.borderColor = Colors::LightGrey;
  spriteProps.borderSize = 1;
  if (!props.characterSpriteName.empty()) {
    spriteProps.bgSprite = props.characterSpriteName;
  }
  sprite->setProps(spriteProps);
  container->getChildren().push_back(std::move(sprite));

  const int statusAreaX = props.padding + spriteSize + props.padding;
  const int statusAreaWidth = std::max(0, style.width - statusAreaX - props.padding);

  int visibleStatusRows = 0;
  for (size_t i = 0; i < props.statusEffectSpriteNames.size(); i++) {
    const int col = static_cast<int>(i) % statusGridCols;
    const int row = static_cast<int>(i) / statusGridCols;

    const int iconX = statusAreaX + col * (statusIconSize + statusIconGap);
    const int iconY = topRowY + row * (statusIconSize + statusIconGap);

    if (iconX + statusIconSize > statusAreaX + statusAreaWidth) {
      continue;
    }

    auto statusIcon = std::make_unique<Quad>(window, container.get());
    BaseStyle statusStyle;
    statusStyle.x = iconX;
    statusStyle.y = iconY;
    statusStyle.width = statusIconSize;
    statusStyle.height = statusIconSize;
    statusStyle.scale = 1.0f;
    statusIcon->setStyle(statusStyle);

    QuadProps statusProps;
    statusProps.bgColor = Colors::ButtonModalGrey2;
    statusProps.borderColor = Colors::White;
    statusProps.borderSize = 1;
    if (!props.statusEffectSpriteNames[i].empty()) {
      statusProps.bgSprite = props.statusEffectSpriteNames[i];
    }
    statusIcon->setProps(statusProps);
    container->getChildren().push_back(std::move(statusIcon));

    visibleStatusRows = std::max(visibleStatusRows, row + 1);
  }

  int statusGridHeight = 0;
  if (visibleStatusRows > 0) {
    statusGridHeight =
        visibleStatusRows * statusIconSize + (visibleStatusRows - 1) * statusIconGap;
  }

  const int topRowHeight = std::max(spriteSize, statusGridHeight);
  const int statRowY = topRowY + topRowHeight + props.rowGap;

  auto healthText = std::make_unique<TextLine>(window, container.get());
  BaseStyle healthStyle;
  healthStyle.x = props.padding;
  healthStyle.y = statRowY;
  healthStyle.fontFamily = FontFamily::PARAGRAPH;
  healthStyle.fontSize = sdl2w::TEXT_SIZE_16;
  healthStyle.fontColor = Colors::Red;
  healthStyle.textAlign = TextAlign::LEFT_TOP;
  healthText->setStyle(healthStyle);
  TextLineProps healthProps;
  TextBlock healthBlock;
  healthBlock.text = "HP: " + std::to_string(props.health);
  healthProps.textBlocks.push_back(healthBlock);
  healthText->setProps(healthProps);
  container->getChildren().push_back(std::move(healthText));

  auto manaText = std::make_unique<TextLine>(window, container.get());
  BaseStyle manaStyle;
  manaStyle.x = style.width / 2;
  manaStyle.y = statRowY;
  manaStyle.fontFamily = FontFamily::PARAGRAPH;
  manaStyle.fontSize = sdl2w::TEXT_SIZE_16;
  manaStyle.fontColor = Colors::Blue;
  manaStyle.textAlign = TextAlign::LEFT_TOP;
  manaText->setStyle(manaStyle);
  TextLineProps manaProps;
  TextBlock manaBlock;
  manaBlock.text = "MP: " + std::to_string(props.mana);
  manaProps.textBlocks.push_back(manaBlock);
  manaText->setProps(manaProps);
  container->getChildren().push_back(std::move(manaText));

  children.push_back(std::move(container));
}

void GameChStatus::render(int dt) { UiElement::render(dt); }

} // namespace ui
