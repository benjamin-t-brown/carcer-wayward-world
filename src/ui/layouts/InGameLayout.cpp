#include "InGameLayout.h"
#include "ui/components/BorderInGameNarrow.h"
#include "ui/components/BorderInGameWide.h"
#include "ui/elements/Quad.h"
#include "ui/elements/buttons/ButtonWorldAction.h"

namespace ui {

namespace {

template <typename BorderT>
void applyTitleLayout(UiElement* titleElement, BorderT* border, float layoutScale) {
  if (!titleElement || !border) {
    return;
  }

  auto& titleStyle = titleElement->getStyle();
  auto [titleX, titleY] = border->getTitleLocation();
  auto [_, titleBarHeight] = border->getTitleDims();
  auto [__, titleHeight] = titleElement->getDims();
  titleStyle.x = titleX + 8 * layoutScale;
  titleStyle.y = titleY + (titleBarHeight - titleHeight) / 2;
  titleElement->build();
}

void buildActionButtons(const std::pair<int, int>& actionButtonsAreaLocation,
                        int actionButtonsQuadWidth,
                        int actionButtonsQuadHeight,
                        Quad* actionButtonsQuad,
                        const InGameLayoutProps& layoutProps,
                        float layoutScale) {
  auto& actionButtonsStyle = actionButtonsQuad->getStyle();
  actionButtonsStyle.x = actionButtonsAreaLocation.first;
  actionButtonsStyle.y = actionButtonsAreaLocation.second;
  actionButtonsStyle.width = actionButtonsQuadWidth;
  actionButtonsStyle.height = actionButtonsQuadHeight;
  actionButtonsStyle.scale = layoutScale;

  auto buttonWidthScaled = static_cast<int>(ACTION_BUTTON_SIZE * layoutProps.actionButtonScale);
  auto buttonSmallIndex = 0;
  auto xAgg = 0;
  auto xAdditionalOffset = 0;
  for (int i = 0; i < static_cast<int>(layoutProps.worldActionTypes.size()); i++) {
    if (i == 1) {
      xAdditionalOffset += buttonWidthScaled / 4;
    }
    auto worldActionType = layoutProps.worldActionTypes[i];
    auto button = new ButtonWorldAction(actionButtonsQuad->getWindow(), actionButtonsQuad);
    auto isSmall = ButtonWorldAction::checkIfWorldActionButtonIsSmall(worldActionType);

    auto& buttonStyle = button->getStyle();
    buttonStyle.x = xAgg + xAdditionalOffset;
    buttonStyle.y = 0;
    buttonStyle.scale = layoutProps.actionButtonScale;

    xAgg += buttonWidthScaled;
    if (isSmall) {
      if (buttonSmallIndex == 0) {
        xAgg -= buttonWidthScaled;
        buttonSmallIndex = 1;
      } else {
        buttonStyle.y += buttonWidthScaled / 2;
        buttonSmallIndex = 0;
      }
    }

    button->setId("worldActionButton_" + std::to_string(i));
    button->setProps(ButtonWorldActionProps{worldActionType});
    actionButtonsQuad->addChild(button);
  }

  actionButtonsQuad->build();
}

} // namespace

InGameLayout::InGameLayout(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {}

void InGameLayout::setProps(const InGameLayoutProps& _props) {
  props = _props;
  build();
}

InGameLayoutProps& InGameLayout::getProps() { return props; }

const InGameLayoutProps& InGameLayout::getProps() const { return props; }

void InGameLayout::setTitleElement(UiElement* _titleElement) {
  removeChildById("title");
  if (!_titleElement) {
    return;
  }

  if (props.borderType == InGameBorderType::Wide) {
    applyTitleLayout(_titleElement,
                     dynamic_cast<BorderInGameWide*>(getChildById("border")),
                     style.scale);
  } else {
    applyTitleLayout(_titleElement,
                     dynamic_cast<BorderInGameNarrow*>(getChildById("border")),
                     style.scale);
  }

  _titleElement->setId("title");
  addChild(_titleElement);
}

UiElement* InGameLayout::getTitleElement() { return getChildById("title"); }

void InGameLayout::build() {
  removeChildById("border");
  removeChildById("actionButtons");

  std::pair<int, int> actionButtonsAreaLocation;
  int actionButtonsQuadWidth = 0;
  int actionButtonsQuadHeight = 0;

  if (props.borderType == InGameBorderType::Wide) {
    auto wideBorder = new BorderInGameWide(window, this);
    wideBorder->setId("border");
    auto& borderStyle = wideBorder->getStyle();
    borderStyle.x = style.x;
    borderStyle.y = style.y;
    borderStyle.width = style.width;
    borderStyle.height = style.height;
    borderStyle.scale = style.scale;
    auto borderProps = BorderInGameWideProps{};
    borderProps.actionButtonsScale = props.actionButtonScale;
    wideBorder->setProps(borderProps);
    actionButtonsAreaLocation = wideBorder->getActionButtonsAreaLocation();
    actionButtonsQuadWidth = style.width - borderProps.leftBorderWidth -
                             borderProps.partyMemberAreaWidth -
                             2 * borderProps.outsetBorderSize;
    actionButtonsQuadHeight = ACTION_BUTTON_SIZE * 2 * borderProps.actionButtonsScale +
                              borderProps.outsetBorderSize * 2;
    children.insert(children.begin(), std::unique_ptr<UiElement>(wideBorder));
  } else {
    auto narrowBorder = new BorderInGameNarrow(window, this);
    narrowBorder->setId("border");
    auto& borderStyle = narrowBorder->getStyle();
    borderStyle.x = style.x;
    borderStyle.y = style.y;
    borderStyle.width = style.width;
    borderStyle.height = style.height;
    borderStyle.scale = style.scale;
    auto borderProps = BorderInGameNarrowProps{};
    borderProps.actionButtonsScale = props.actionButtonScale;
    narrowBorder->setProps(borderProps);
    actionButtonsAreaLocation = narrowBorder->getActionButtonsAreaLocation();
    actionButtonsQuadWidth =
        style.width - borderProps.sideBorderWidth * 2 - 2 * borderProps.outsetBorderSize;
    actionButtonsQuadHeight = ACTION_BUTTON_SIZE * 2 * borderProps.actionButtonsScale +
                              borderProps.outsetBorderSize * 2;
    children.insert(children.begin(), std::unique_ptr<UiElement>(narrowBorder));
  }

  auto actionButtonsQuad = new Quad(window, this);
  actionButtonsQuad->setId("actionButtons");
  addChild(actionButtonsQuad);
  buildActionButtons(actionButtonsAreaLocation,
                     actionButtonsQuadWidth,
                     actionButtonsQuadHeight,
                     actionButtonsQuad,
                     props,
                     style.scale);

  if (auto* titleElement = getTitleElement()) {
    if (props.borderType == InGameBorderType::Wide) {
      applyTitleLayout(titleElement,
                       dynamic_cast<BorderInGameWide*>(getChildById("border")),
                       style.scale);
    } else {
      applyTitleLayout(titleElement,
                       dynamic_cast<BorderInGameNarrow*>(getChildById("border")),
                       style.scale);
    }
  }
}

void InGameLayout::render(int dt) { UiElement::render(dt); }

} // namespace ui
