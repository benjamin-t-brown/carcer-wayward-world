#include "InGameLayout.h"
#include "ui/components/borders/BorderInGame.h"
#include "ui/components/borders/BorderInGameNarrow.h"
#include "ui/components/borders/BorderInGameWide.h"
#include "ui/components/lists/ListChCompactInfoHorizontal.h"
#include "ui/components/lists/ListChCompactInfoVertical.h"
#include "ui/elements/Quad.h"
#include "ui/elements/buttons/ButtonClose.h"
#include "ui/elements/buttons/ButtonWorldAction.h"
#include "ui/elements/TextLine.h"
#include "ui/components/InGameTitleBar.h"
#include "ui/colors.h"

namespace ui {

InGameLayout::InGameLayout(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {}

void InGameLayout::setProps(const InGameLayoutProps& _props) {
  props = _props;
  if (props.width > 0) {
    style.width = props.width;
  }
  if (props.height > 0) {
    style.height = props.height;
  }
  build();
}

InGameLayoutProps& InGameLayout::getProps() { return props; }

const InGameLayoutProps& InGameLayout::getProps() const { return props; }

void InGameLayout::applyTitleLayout(UiElement* titleElement, BorderInGame* border) {
  if (!titleElement || !border) {
    return;
  }

  auto [titleX, titleY] = border->getTitleLocation();
  auto [titleWidth, titleBarHeight] = border->getTitleDims();
  titleElement->setPos(titleX, titleY);
  titleElement->setScale(style.scale);
  if (auto* titleBar = dynamic_cast<InGameTitleBar*>(titleElement)) {
    auto titleProps = titleBar->getProps();
    titleProps.width = static_cast<int>(titleWidth / style.scale);
    titleProps.height = static_cast<int>(titleBarHeight / style.scale);
    titleBar->setProps(titleProps);
  } else {
    titleElement->build();
  }
}

void InGameLayout::buildActionButtons(const std::pair<int, int>& actionButtonsAreaLocation,
                                      int actionButtonsQuadWidth,
                                      int actionButtonsQuadHeight,
                                      Quad* actionButtonsQuad) {
  actionButtonsQuad->setPos(actionButtonsAreaLocation.first,
                            actionButtonsAreaLocation.second);
  actionButtonsQuad->setScale(style.scale);
  actionButtonsQuad->setProps(QuadProps{
      .width = actionButtonsQuadWidth,
      .height = actionButtonsQuadHeight,
  });

  auto buttonWidthScaled =
      static_cast<int>(BorderInGame::ACTION_BUTTON_SIZE * props.actionButtonScale);
  auto buttonSmallIndex = 0;
  auto xAgg = 0;
  auto xAdditionalOffset = 0;
  for (int i = 0; i < static_cast<int>(props.worldActionTypes.size()); i++) {
    if (i == 1) {
      xAdditionalOffset += buttonWidthScaled / 4;
    }
    auto worldActionType = props.worldActionTypes[i];
    auto button =
        new ButtonWorldAction(actionButtonsQuad->getWindow(), actionButtonsQuad);
    auto isSmall = ButtonWorldAction::checkIfWorldActionButtonIsSmall(worldActionType);

    int buttonX = xAgg + xAdditionalOffset;
    int buttonY = 0;

    xAgg += buttonWidthScaled;
    if (isSmall) {
      if (buttonSmallIndex == 0) {
        xAgg -= buttonWidthScaled;
        buttonSmallIndex = 1;
      } else {
        buttonY += buttonWidthScaled / 2;
        buttonSmallIndex = 0;
      }
    }

    button->setId("worldActionButton_" + bmin::toString(i));
    button->setPos(buttonX, buttonY);
    button->setScale(props.actionButtonScale);
    button->setProps(ButtonWorldActionProps{worldActionType});
    actionButtonsQuad->addChild(button);
  }

  actionButtonsQuad->build();
}

void InGameLayout::buildChList(const std::pair<int, int>& chListLocation) {
  if (props.partyMembers.empty()) {
    return;
  }

  auto [chListX, chListY] = chListLocation;

  if (props.borderType == InGameBorderType::Wide) {
    auto chList = new ListChCompactInfoVertical(window, this);
    chList->setId("chList");
    chList->setPos(chListX, chListY);
    chList->setScale(style.scale);
    chList->setProps(ListChCompactInfoVerticalProps{
        .entries = props.partyMembers,
        .lineGap = 6,
    });
    addChild(chList);
  } else {
    auto chList = new ListChCompactInfoHorizontal(window, this);
    chList->setId("chList");
    chList->setPos(chListX, chListY);
    chList->setScale(style.scale);
    chList->setProps(ListChCompactInfoHorizontalProps{
        .entries = props.partyMembers,
        .lineGap = 6,
    });
    addChild(chList);
  }
}

void InGameLayout::setTitleElement(UiElement* _titleElement) {
  removeChildById("title");
  if (!_titleElement) {
    return;
  }

  applyTitleLayout(_titleElement,
                   dynamic_cast<BorderInGame*>(getChildById("border")));

  _titleElement->setId("title");
  addChild(_titleElement);
}

UiElement* InGameLayout::getTitleElement() { return getChildById("title"); }

void InGameLayout::setActionModeCancelVisible(bool visible,
                                              const bmin::String& modeLabel) {
  removeChildById("actionModeCancel");
  removeChildById("actionModeLabel");
  if (!visible) {
    return;
  }

  constexpr int kPad = 4;
  constexpr int kLabelGap = 8;
  auto [worldX, worldY] = getWorldLocation();
  auto [worldW, worldH] = getWorldDims();
  (void)worldW;

  auto* cancelButton = new ButtonClose(window, this);
  cancelButton->setId("actionModeCancel");
  cancelButton->setScale(style.scale);
  const int buttonSize = static_cast<int>(ButtonClose::closeButtonSize * style.scale);
  const int buttonX = worldX + kPad;
  const int buttonY = worldY + worldH - buttonSize - kPad;
  cancelButton->setPos(buttonX, buttonY);
  cancelButton->setProps(ButtonCloseProps{
      .closeType = CloseType::MODAL,
  });
  addChild(cancelButton);

  if (!modeLabel.empty()) {
    auto* label = new TextLine(window, this);
    label->setId("actionModeLabel");
    label->setScale(style.scale);
    label->setPos(buttonX + buttonSize + kLabelGap, buttonY + buttonSize / 2);
    label->setProps(TextLineProps{
        .textBlocks = {{.text = modeLabel}},
        .fontFamily = FontFamily::TEXT,
        .fontSize = sdl2w::TEXT_SIZE_16,
        .fontColor = Colors::White,
        .textAlign = TextAlign::LEFT_CENTER,
    });
    addChild(label);
  }
}

const std::pair<int, int> InGameLayout::getWorldDims() {
  auto* border = dynamic_cast<BorderInGame*>(getChildById("border"));
  if (border) {
    return border->getContentDims();
  }
  return {style.width, style.height};
}

const std::pair<int, int> InGameLayout::getWorldLocation() {
  auto* border = dynamic_cast<BorderInGame*>(getChildById("border"));
  if (border) {
    return border->getContentAreaLocation();
  }
  return {style.x, style.y};
}

const std::pair<int, int> InGameLayout::getChListLocation() {
  auto* border = dynamic_cast<BorderInGame*>(getChildById("border"));
  if (border) {
    return border->getPartyMemberAreaLocation();
  }
  return {0, 0};
}

void InGameLayout::build() {
  removeChildById("border");
  removeChildById("actionButtons");
  removeChildById("chList");
  removeChildById("actionModeCancel");
  removeChildById("actionModeLabel");

  if (props.width > 0) {
    style.width = props.width;
  }
  if (props.height > 0) {
    style.height = props.height;
  }

  std::pair<int, int> actionButtonsAreaLocation;
  int actionButtonsQuadWidth = 0;
  int actionButtonsQuadHeight = 0;

  if (props.borderType == InGameBorderType::Wide) {
    auto wideBorder = new BorderInGameWide(window, this);
    wideBorder->setId("border");
    wideBorder->setPos(style.x, style.y);
    wideBorder->setScale(style.scale);
    auto borderProps = BorderInGameWideProps{};
    borderProps.width = style.width;
    borderProps.height = style.height;
    borderProps.actionButtonsScale = props.actionButtonScale;
    wideBorder->setProps(borderProps);
    actionButtonsAreaLocation = wideBorder->getActionButtonsAreaLocation();
    actionButtonsQuadWidth = style.width - borderProps.leftBorderWidth -
                             borderProps.partyMemberAreaWidth -
                             2 * borderProps.outsetBorderSize;
    actionButtonsQuadHeight = BorderInGame::ACTION_BUTTON_SIZE * 2 * borderProps.actionButtonsScale +
                              borderProps.outsetBorderSize * 2;
    children.insert(children.begin(), bmin::UniquePtr<UiElement>(wideBorder));
  } else {
    auto narrowBorder = new BorderInGameNarrow(window, this);
    narrowBorder->setId("border");
    narrowBorder->setPos(style.x, style.y);
    narrowBorder->setScale(style.scale);
    auto borderProps = BorderInGameNarrowProps{};
    borderProps.width = style.width;
    borderProps.height = style.height;
    borderProps.actionButtonsScale = props.actionButtonScale;
    narrowBorder->setProps(borderProps);
    actionButtonsAreaLocation = narrowBorder->getActionButtonsAreaLocation();
    actionButtonsQuadWidth =
        style.width - borderProps.sideBorderWidth * 2 - 2 * borderProps.outsetBorderSize;
    actionButtonsQuadHeight = BorderInGame::ACTION_BUTTON_SIZE * 2 * borderProps.actionButtonsScale +
                              borderProps.outsetBorderSize * 2;
    children.insert(children.begin(), bmin::UniquePtr<UiElement>(narrowBorder));
  }

  auto actionButtonsQuad = new Quad(window, this);
  actionButtonsQuad->setId("actionButtons");
  addChild(actionButtonsQuad);
  buildActionButtons(actionButtonsAreaLocation,
                     actionButtonsQuadWidth,
                     actionButtonsQuadHeight,
                     actionButtonsQuad);

  buildChList(getChListLocation());

  if (auto* titleElement = getTitleElement()) {
    applyTitleLayout(titleElement,
                     dynamic_cast<BorderInGame*>(getChildById("border")));
  }
}

void InGameLayout::render(int dt) { UiElement::render(dt); }

} // namespace ui
