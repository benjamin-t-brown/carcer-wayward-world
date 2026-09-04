module;
#include <cstddef>
#include <cstdint>
#include <utility>

export module carcer.ui.components.InGameTitleBar;
export import bmin.containers;
import bmin.string_interop;
export import carcer.ui.UiElement;
import sdl2w;
import carcer.ui.elements;
import carcer.ui.TextStyle;
#include "macros.h"

export {

// --- from ui/components/InGameTitleBar.h ---
namespace ui {

struct InGameTitleBarProps {
  int width = 0;
  int height = 0;
  bmin::String title = "Title";
  int day = 0;
  int food = 0;
  int ap = 0;
  bool showAp = false;

  int buttonSize = 32;
  int buttonSpacing = 0;
  int sectionSpacing = 12;
  int statSpacing = 16;
};

// InGameTitleBar component - title bar for the in-game layout with menu buttons,
// customizable title text, and day/food/ap stats.
class InGameTitleBar : public UiElement {
private:
  InGameTitleBarProps props;

  int getContentHeight() const;
  int getBarCenterY() const;
  int centerTopY(int elementHeight) const;

public:
  InGameTitleBar(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~InGameTitleBar() override = default;

  void setProps(const InGameTitleBarProps& _props);
  InGameTitleBarProps& getProps();
  const InGameTitleBarProps& getProps() const;

  // void addMenuButtonObserver(UiEventObserver* observer);
  // void addHelpButtonObserver(UiEventObserver* observer);

  UiElement* createStatLineRightAligned(const bmin::String& text, int x, int y);

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

namespace ui {

InGameTitleBar::InGameTitleBar(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {}

void InGameTitleBar::setProps(const InGameTitleBarProps& _props) {
  props = _props;
  if (props.width > 0) {
    style.width = props.width;
  }
  if (props.height > 0) {
    style.height = props.height;
  }
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
  TextFontProps font;
  setBaseFontConfig(font, BaseFontConfig::MODAL_TEXT_BOLD);
  TextLineProps statLineProps;
  statLineProps.fontFamily = font.fontFamily;
  statLineProps.fontSize = font.fontSize;
  statLineProps.fontColor = font.fontColor;
  statLineProps.textAlign = TextAlign::LEFT_CENTER;
  statLineProps.textBlocks.pushBack(TextBlock{.text = text});
  statLine->setPos(style.x, style.y);
  statLine->setScale(1.f);
  statLine->setProps(statLineProps);
  auto [statWidth, _] = statLine->getDims();
  statLine->setPos(x - statWidth, y);
  return statLine;
}

void InGameTitleBar::build() {
  children.clear();

  if (props.width > 0) {
    style.width = props.width;
  }
  if (props.height > 0) {
    style.height = props.height;
  }

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
  menuButton->setPos(cursorX, buttonTopY);
  menuButton->setScale(style.scale);
  menuButton->setProps(ButtonIconProps{
      .regularSprite = ButtonIcon::HAMBURGER_ICON1,
      .activeSprite = ButtonIcon::HAMBURGER_ICON2,
      .iconSize = props.buttonSize,
  });
  addChild(menuButton);
  cursorX += scaledButtonSize + scaledButtonSpacing;

  auto helpButton = new ButtonIcon(window, this);
  helpButton->setId("helpButton");
  helpButton->setPos(cursorX, buttonTopY);
  helpButton->setScale(style.scale);
  helpButton->setProps(ButtonIconProps{
      .regularSprite = ButtonIcon::QUESTION_ICON1,
      .activeSprite = ButtonIcon::QUESTION_ICON2,
      .iconSize = props.buttonSize,
  });
  addChild(helpButton);
  cursorX += scaledButtonSize + scaledSectionSpacing;

  TextFontProps titleFont;
  setBaseFontConfig(titleFont, BaseFontConfig::MODAL_TITLE);

  auto titleText = new TextLine(window, this);
  titleText->setId("titleText");
  titleText->setPos(cursorX, barCenterY);
  titleText->setScale(1.f);
  TextLineProps titleProps;
  titleProps.fontFamily = titleFont.fontFamily;
  titleProps.fontSize = titleFont.fontSize;
  titleProps.fontColor = titleFont.fontColor;
  titleProps.textAlign = TextAlign::LEFT_CENTER;
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
