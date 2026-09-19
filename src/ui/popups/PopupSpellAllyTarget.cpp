#include "PopupSpellAllyTarget.h"
#include "sdl2w/L10n.h"
#include "ui/colors.hpp"
#include "ui/components/borders/BorderDropShadow.h"
#include "ui/components/lists/ListSpellAllyTargets.h"
#include "ui/elements/TextLine.h"
#include "ui/elements/buttons/ButtonClose.h"
#include "ui/observers/ActionObserver.hpp"
#include "actions/navigation/UiRemoveLayer.hpp"

namespace ui {

PopupSpellAllyTarget::PopupSpellAllyTarget(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  shouldPropagateEventsToChildren = true;
}

void PopupSpellAllyTarget::setProps(const PopupSpellAllyTargetProps& _props) {
  props = _props;
  build();
}

PopupSpellAllyTargetProps& PopupSpellAllyTarget::getProps() { return props; }

const PopupSpellAllyTargetProps& PopupSpellAllyTarget::getProps() const { return props; }

void PopupSpellAllyTarget::build() {
  children.clear();

  style.width = 320;
  const int padding = 8;
  const int closeButtonSize = 32;
  const int scaledWidth = static_cast<int>(style.width * style.scale);
  const int paddingScaled = static_cast<int>(padding * style.scale);
  const int contentWidth = style.width - 2 * padding;

  auto closeButton = new ButtonClose(window, this);
  closeButton->setId("closeButton");
  closeButton->setPos(
      style.x + scaledWidth - closeButtonSize * style.scale - padding * style.scale,
      style.y + padding * style.scale);
  closeButton->setScale(style.scale);
  closeButton->setProps(ButtonCloseProps{.closeType = CloseType::POPUP});
  closeButton->addEventObserver(ui::makeActionObserver<state::actions::UiRemoveLayer>(
      state::LayerId::SpellAllyTarget));
  addChild(bmin::UniquePtr<ui::UiElement>(closeButton));

  auto title = new TextLine(window, this);
  title->setId("title");
  TextFontProps titleFont;
  setBaseFontConfig(titleFont, BaseFontConfig::MODAL_TITLE);
  title->setPos(style.x + padding * style.scale,
                style.y + padding * style.scale + (closeButtonSize * style.scale) / 2);
  title->setScale(1.f);
  title->setProps(TextLineProps{
      .textBlocks =
          {
              {
                  .text = props.titleText,
              },
          },
      .fontFamily = titleFont.fontFamily,
      .fontSize = titleFont.fontSize,
      .fontColor = Colors::Black,
      .textAlign = TextAlign::LEFT_CENTER,
  });
  addChild(bmin::UniquePtr<ui::UiElement>(title));
  auto [titleWidth, titleHeight] = title->calculateTextDims();
  (void)titleWidth;

  int contentY = style.y + paddingScaled + titleHeight + paddingScaled * 2;

  if (!props.statusText.empty()) {
    auto statusText = new TextLine(window, this);
    statusText->setId("StatusText");
    TextFontProps statusFont;
    setBaseFontConfig(statusFont, BaseFontConfig::MODAL_TEXT);
    statusText->setPos(style.x + padding * style.scale, contentY);
    statusText->setScale(1.f);
    statusText->setProps(TextLineProps{
        .textBlocks =
            {
                {
                    .text = props.statusText,
                },
            },
        .fontFamily = statusFont.fontFamily,
        .fontSize = statusFont.fontSize,
        .fontColor = Colors::DarkGrey,
        .textAlign = TextAlign::LEFT_TOP,
    });
    addChild(bmin::UniquePtr<ui::UiElement>(statusText));
    auto [statusW, statusH] = statusText->calculateTextDims();
    (void)statusW;
    contentY += statusH + paddingScaled;
  }

  auto listAllies = new ListSpellAllyTargets(window, this);
  listAllies->setId("listAllies");
  listAllies->setPos(style.x + padding * style.scale, contentY);
  listAllies->setScale(style.scale);

  ListSpellAllyTargetsProps listProps;
  listProps.casterId = props.casterId;
  listProps.spellId = props.spellId;
  listProps.width = contentWidth;
  listProps.paddingTop = 0;
  listProps.paddingBottom = 4;
  for (const auto& ally : props.allies) {
    listProps.allies.pushBack(ListSpellAllyTargetsEntry{
        .id = ally.id,
        .label = ally.label,
        .iconSprite = ally.iconSprite,
    });
  }
  listAllies->setProps(listProps);
  auto [listW, listH] = listAllies->getDims();
  (void)listW;
  addChild(bmin::UniquePtr<ui::UiElement>(listAllies));

  style.height = ((contentY + listH + paddingScaled) - style.y) / style.scale;

  auto border = new BorderDropShadow(window, this);
  border->setPos(style.x, style.y);
  border->setScale(style.scale);
  border->setProps(BorderDropShadowProps{
      .width = style.width,
      .height = style.height,
  });
  children.insert(children.begin(), bmin::UniquePtr<UiElement>(border));
}

void PopupSpellAllyTarget::render(int dt) { UiElement::render(dt); }

} // namespace ui
