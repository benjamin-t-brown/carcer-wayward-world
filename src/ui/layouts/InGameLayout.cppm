module;
#include <cstddef>
#include <cstdint>
#include <utility>
#if __has_include(<SDL.h>)
#include <SDL.h>
#include <SDL_pixels.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_pixels.h>
#endif

export module carcer.ui.layouts:InGameLayout;
export import bmin.containers;
import bmin.string_interop;
export import carcer.state;
export import carcer.ui.core;
export import carcer.ui.components;
export import carcer.ui.elements;
import sdl2w;
import carcer.actions;
import carcer.ui.helpers;
import carcer.ui.components;
import carcer.ui.elements;
import carcer.ui.lists;
import carcer.ui.core;
#include "macros.h"

export {

// --- from ui/layouts/InGameLayout.h ---
// IWYU pragma: keep

namespace ui {


enum class InGameBorderType { Wide, Narrow };

// InGameLayout layout properties
struct InGameLayoutProps {
  int width = 0;
  int height = 0;
  bmin::DynArray<state::WorldActionType> worldActionTypes;
  bmin::DynArray<ChCompactInfoProps> partyMembers;
  int selectedPartyMemberIndex = 0;
  float actionButtonScale = 1.f;
  InGameBorderType borderType = InGameBorderType::Wide;
};

// InGameLayout layout - renders an in-game layout with background, border, title,
// subtitle, and children Uses Position, Size, Scale from BaseStyle
class InGameLayout : public UiElement {
private:
  InGameLayoutProps props;

  void applyTitleLayout(UiElement* titleElement, BorderInGame* border);
  void buildActionButtons(const std::pair<int, int>& actionButtonsAreaLocation,
                          int actionButtonsQuadWidth,
                          int actionButtonsQuadHeight,
                          Quad* actionButtonsQuad);
  void buildChList(const std::pair<int, int>& chListLocation);

public:
  InGameLayout(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~InGameLayout() override = default;

  void setProps(const InGameLayoutProps& _props);
  InGameLayoutProps& getProps();
  const InGameLayoutProps& getProps() const;

  void setTitleElement(UiElement* _titleElement);
  UiElement* getTitleElement();
  const std::pair<int, int> getWorldDims();
  const std::pair<int, int> getWorldLocation();
  const std::pair<int, int> getChListLocation();

  // Modal-style X + mode label in the bottom-left of the map area.
  void setActionModeCancelVisible(bool visible, const bmin::String& modeLabel = {});

  void build() override;
  void render(int dt) override;
};

} // namespace ui

// --- from ui/observers/ObserverCancelWorldActionMode.hpp ---
namespace ui {

class ObserverCancelWorldActionMode : public UiEventObserver {
  state::StateManager* stateManager;

public:
  explicit ObserverCancelWorldActionMode(state::StateManager* _stateManager)
      : stateManager(_stateManager) {}

  void onClick(int /*mouseX*/, int /*mouseY*/, int /*button*/) override;
};

} // namespace ui

// --- from ui/observers/ObserverWorldAction.hpp ---
namespace ui {

class ObserverWorldAction : public UiEventObserver {
  state::StateManager* stateManager;
  state::WorldActionType worldActionType;
  sdl2w::Window* window;

public:
  ObserverWorldAction(state::StateManager* _stateManager,
                      state::WorldActionType _worldActionType,
                      sdl2w::Window* _window)
      : stateManager(_stateManager),
        worldActionType(_worldActionType),
        window(_window) {}

  void onClick(int mouseX, int mouseY, int button) override;
};

} // namespace ui

// --- from ui/observers/ObserverSetSelectedPartyMemberId.hpp ---
namespace ui {

class ObserverSetSelectedPartyMemberId : public ui::UiEventObserver,
                                         public state::StateManagerInterface {
  bmin::String partyMemberId;

public:
  explicit ObserverSetSelectedPartyMemberId(bmin::String _partyMemberId)
      : partyMemberId(std::move(_partyMemberId)) {}

  void onClick(int /*mouseX*/, int /*mouseY*/, int /*button*/) override;
};

} // namespace ui

} // export

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
        .selectedIndex = props.selectedPartyMemberIndex,
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
        .selectedIndex = props.selectedPartyMemberIndex,
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
  addChild(cancelButton);

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
    addChild(labelBg);
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
  removeChildById("actionModeLabelBg");
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

namespace ui {

void ObserverCancelWorldActionMode::onClick(int /*mouseX*/, int /*mouseY*/, int /*button*/) {
    if (stateManager) {
      cancelCurrentWorldActionMode(*stateManager);
    }
  }

void ObserverWorldAction::onClick(int mouseX, int mouseY, int button) {
    if (stateManager) {
      activateWorldAction(*stateManager, worldActionType, window);
    }
  }

void ObserverSetSelectedPartyMemberId::onClick(int /*mouseX*/, int /*mouseY*/, int /*button*/) {
    auto stateManager = getStateManager();
    if (!stateManager) {
      return;
    }
    // Party member switching is locked while combat is active.
    if (stateManager->getState().world.combat.active) {
      return;
    }
    LOG(INFO) << "ObserverSetSelectedPartyMemberId::onClick id=" << partyMemberId
              << LOG_ENDL;
    stateManager->enqueueAction(
        stateManager->getActionData(),
        state::actions::setSelectedPartyMemberId(partyMemberId),
        0);
  }

} // namespace ui
