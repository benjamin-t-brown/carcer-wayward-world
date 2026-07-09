#include "InGameTitleBar.h"
#include "lib/sdl2w/L10n.h"
#include "ui/elements/TextLine.h"
#include "ui/elements/buttons/ButtonIcon.h"

namespace ui {

InGameTitleBar::InGameTitleBar(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {}

void InGameTitleBar::setProps(const InGameTitleBarProps& _props) {
  props = _props;
  build();
}

InGameTitleBarProps& InGameTitleBar::getProps() { return props; }

const InGameTitleBarProps& InGameTitleBar::getProps() const { return props; }

int InGameTitleBar::getContentHeight() const { return props.buttonSize; }

int InGameTitleBar::getBarCenterY() const {
  return style.y + static_cast<int>(style.height * style.scale) / 2;
}

int InGameTitleBar::centerTopY(int elementHeight) const {
  return getBarCenterY() - elementHeight / 2;
}

UiElement*
InGameTitleBar::createStatLineRightAligned(const bmin::String& text, int x, int y) {
  auto statLine = new TextLine(window, this);
  auto& statStyle = statLine->getStyle();
  statStyle.x = style.x;
  statStyle.y = style.y;
  statStyle.scale = 1.f;
  setBaseFontConfig(statStyle, BaseFontConfig::MODAL_TEXT_BOLD);
  statStyle.textAlign = TextAlign::LEFT_CENTER;
  TextLineProps statLineProps;
  statLineProps.textBlocks.pushBack(TextBlock{.text = text});
  statLine->setProps(statLineProps);
  auto [statWidth, _] = statLine->getDims();
  statStyle.x = x - statWidth;
  statStyle.y = y;
  statLine->build();
  return statLine;
}

void InGameTitleBar::build() {
  children.clear();

  const int scaledButtonSize = static_cast<int>(props.buttonSize * style.scale);
  const int scaledButtonSpacing = static_cast<int>(props.buttonSpacing * style.scale);
  const int scaledSectionSpacing = static_cast<int>(props.sectionSpacing * style.scale);
  const int scaledStatSpacing = static_cast<int>(props.statSpacing * style.scale);
  const int barCenterY = getBarCenterY();
  const int buttonTopY = centerTopY(scaledButtonSize);
  auto [scaledWidth, scaledHeight] = getDims();

  int cursorX = style.x;

  auto menuButton = new ButtonIcon(window, this);
  menuButton->setId("menuButton");
  auto& menuStyle = menuButton->getStyle();
  menuStyle.x = cursorX;
  menuStyle.y = buttonTopY;
  menuStyle.scale = style.scale;
  menuButton->setProps(ButtonIconProps{
      .regularSprite = ButtonIcon::HAMBURGER_ICON1,
      .activeSprite = ButtonIcon::HAMBURGER_ICON2,
      .iconSize = props.buttonSize,
  });
  addChild(menuButton);
  cursorX += scaledButtonSize + scaledButtonSpacing;

  auto helpButton = new ButtonIcon(window, this);
  helpButton->setId("helpButton");
  auto& helpStyle = helpButton->getStyle();
  helpStyle.x = cursorX;
  helpStyle.y = buttonTopY;
  helpStyle.scale = style.scale;
  helpButton->setProps(ButtonIconProps{
      .regularSprite = ButtonIcon::QUESTION_ICON1,
      .activeSprite = ButtonIcon::QUESTION_ICON2,
      .iconSize = props.buttonSize,
  });
  addChild(helpButton);
  cursorX += scaledButtonSize + scaledSectionSpacing;

  auto titleText = new TextLine(window, this);
  titleText->setId("titleText");
  auto& titleStyle = titleText->getStyle();
  titleStyle.x = cursorX;
  titleStyle.y = barCenterY;
  titleStyle.scale = 1.f;
  setBaseFontConfig(titleStyle, BaseFontConfig::MODAL_TITLE);
  titleStyle.textAlign = TextAlign::LEFT_CENTER;
  TextLineProps titleProps;
  titleProps.textBlocks.pushBack(TextBlock{.text = props.title});
  titleText->setProps(titleProps);
  addChild(titleText);

  int statX = style.x + scaledWidth - scaledStatSpacing;
  auto* dayStatLine = createStatLineRightAligned(
      TRANSLATE("Day: ") + bmin::toString(props.day), statX, barCenterY);
  addChild(dayStatLine);
  auto [dayStatWidth, _] = dayStatLine->getDims();
  statX -= dayStatWidth + scaledStatSpacing;
  auto* foodStatLine = createStatLineRightAligned(
      TRANSLATE("Food: ") + bmin::toString(props.food), statX, barCenterY);
  addChild(foodStatLine);
  if (props.showAp) {
    auto [foodStatWidth, _] = foodStatLine->getDims();
    statX -= foodStatWidth + scaledStatSpacing;
    auto* apStatLine = createStatLineRightAligned(
        TRANSLATE("AP: ") + bmin::toString(props.ap), statX, barCenterY);
    addChild(apStatLine);
  }
}

void InGameTitleBar::render(int dt) { UiElement::render(dt); }

} // namespace ui
