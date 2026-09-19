#include "InGameLayout.h"
#include "ui/components/borders/BorderInGame.h"
#include "ui/components/borders/BorderInGameNarrow.h"
#include "ui/components/borders/BorderInGameWide.h"
#include "ui/components/lists/ListChCompactInfoHorizontal.h"
#include "ui/components/lists/ListChCompactInfoVertical.h"
#include "ui/elements/OutsetRectangle.h"
#include "ui/elements/Quad.h"
#include "ui/elements/buttons/ButtonClose.h"
#include "ui/elements/buttons/ButtonScroll.h"
#include "ui/elements/buttons/ButtonWorldAction.h"
#include "ui/elements/TextLine.h"
#include "ui/components/InGameTitleBar.h"
#include "ui/colors.hpp"
#include "ui/uiUtils.hpp"

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
    actionButtonsQuad->addChild(bmin::UniquePtr<ui::UiElement>(button));
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
        .selectedIndex = props.selectedPartyMemberIndex,
        .lineGap = 6,
    });
    addChild(bmin::UniquePtr<ui::UiElement>(chList));
  } else {
    auto chList = new ListChCompactInfoHorizontal(window, this);
    chList->setId("chList");
    chList->setPos(chListX, chListY);
    chList->setScale(style.scale);
    chList->setProps(ListChCompactInfoHorizontalProps{
        .entries = props.partyMembers,
        .selectedIndex = props.selectedPartyMemberIndex,
        .lineGap = 6,
    });
    addChild(bmin::UniquePtr<ui::UiElement>(chList));
  }
}

void InGameLayout::setTitleElement(bmin::UniquePtr<UiElement> _titleElement) {
  removeChildById("title");
  if (!_titleElement) {
    return;
  }

  applyTitleLayout(_titleElement.get(),
                   dynamic_cast<BorderInGame*>(getChildById("border")));

  _titleElement->setId("title");
  addChild(bmin::move(_titleElement));
}

UiElement* InGameLayout::getTitleElement() { return getChildById("title"); }

void InGameLayout::setActionModeCancelVisible(bool visible,
                                              const bmin::String& modeLabel) {
  removeChildById("actionModeCancel");
  removeChildById("actionModeLabelBg");
  removeChildById("actionModeLabel");
  if (!visible) {
    return;
  }

  constexpr int kPad = 4;
  constexpr int kLabelGap = 8;
  constexpr int kLabelPad = 5;
  auto [worldX, worldY] = getWorldLocation();
  auto [worldW, worldH] = getWorldDims();

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
  addChild(bmin::UniquePtr<ui::UiElement>(cancelButton));

  if (!modeLabel.empty()) {
    const int labelX = buttonX + buttonSize + kLabelGap;
    const int labelY = buttonY + buttonSize / 2;

    auto* label = new TextLine(window, this);
    label->setId("actionModeLabel");
    label->setScale(style.scale);
    label->setPos(labelX, labelY);
    label->setProps(TextLineProps{
        .textBlocks = {{.text = modeLabel}},
        .fontFamily = FontFamily::TEXT,
        .fontSize = sdl2w::TEXT_SIZE_16,
        .fontColor = Colors::White,
        .textAlign = TextAlign::LEFT_CENTER,
    });

    const auto [labelW, labelH] = label->getDims();
    const int padScaled = static_cast<int>(kLabelPad * style.scale);

    auto* labelBg = new OutsetRectangle(window, this);
    labelBg->setId("actionModeLabelBg");
    labelBg->setPos(labelX - padScaled, labelY - labelH / 2 - padScaled);
    labelBg->setScale(1.f);
    labelBg->setProps(OutsetRectangleProps{
        .width = labelW + padScaled * 2,
        .height = labelH + padScaled * 2,
    });
    addChild(bmin::UniquePtr<ui::UiElement>(labelBg));
    addChild(bmin::UniquePtr<ui::UiElement>(label));
  }
}

namespace {

constexpr const char* kSpellAimCameraButtonIds[] = {
    "spellAimCameraUp",
    "spellAimCameraDown",
    "spellAimCameraLeft",
    "spellAimCameraRight",
};

void removeSpellAimCameraButtons(InGameLayout& layout) {
  for (const auto* id : kSpellAimCameraButtonIds) {
    layout.removeChildById(id);
  }
}

} // namespace

void InGameLayout::setSpellAimCameraButtonsVisible(bool visible) {
  removeSpellAimCameraButtons(*this);
  if (!visible) {
    return;
  }

  constexpr int kPad = 4;
  constexpr int kBtn = 32;
  auto [worldX, worldY] = getWorldLocation();
  auto [worldW, worldH] = getWorldDims();
  const int btnSize = static_cast<int>(kBtn * style.scale);
  const int pad = static_cast<int>(kPad * style.scale);

  auto addButton = [&](const char* id, ScrollDirection direction, int x, int y) {
    auto* button = new ButtonScroll(window, this);
    button->setId(id);
    button->setScale(style.scale);
    button->setPos(x, y);
    button->playClickSound = true;
    button->setProps(ButtonScrollProps{
        .direction = direction,
        .width = kBtn,
        .height = kBtn,
    });
    addChild(bmin::UniquePtr<ui::UiElement>(button));
  };

  addButton("spellAimCameraUp",
            ScrollDirection::UP,
            worldX + (worldW - btnSize) / 2,
            worldY + pad);
  addButton("spellAimCameraDown",
            ScrollDirection::DOWN,
            worldX + (worldW - btnSize) / 2,
            worldY + worldH - btnSize - pad);
  addButton("spellAimCameraLeft",
            ScrollDirection::LEFT,
            worldX + pad,
            worldY + (worldH - btnSize) / 2);
  addButton("spellAimCameraRight",
            ScrollDirection::RIGHT,
            worldX + worldW - btnSize - pad,
            worldY + (worldH - btnSize) / 2);
}

bool InGameLayout::isMapOverlayControlAt(int x, int y) {
  constexpr const char* overlayIds[] = {
      "actionModeCancel",
      "actionModeLabel",
      "actionModeLabelBg",
      "spellAimCameraUp",
      "spellAimCameraDown",
      "spellAimCameraLeft",
      "spellAimCameraRight",
  };
  for (const auto* id : overlayIds) {
    if (auto* child = getChildById(id)) {
      if (isInBoundsScaled(x, y, child)) {
        return true;
      }
    }
  }
  return false;
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
  removeChildById("actionModeLabelBg");
  removeChildById("actionModeLabel");
  removeSpellAimCameraButtons(*this);

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
  addChild(bmin::UniquePtr<ui::UiElement>(actionButtonsQuad));
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
