#include "../../setupTestUi.h"
#include "sdl2w/Draw.h"
#include "sdl2w/Logger.h"
#include "sdl2w/Window.h"
#include "state/WorldActions.h"
#include "ui/SdlPixels.h" // IWYU pragma: keep
#include "ui/UiElement.h"
#include "ui/components/InGameTitleBar.h"
#include "ui/elements/buttons/ButtonModal.h"
#include "ui/layouts/InGameLayout.h"
#include "bmin/DynArray.h"
#include "bmin/String.h"
#include "bmin/UniquePtr.h"

namespace {

void copyActionTypes(bmin::DynArray<state::WorldActionType>& dest,
                     const auto& source) {
  dest.clear();
  for (const auto& type : source) {
    dest.pushBack(type);
  }
}

}  // namespace

bmin::DynArray<bmin::UniquePtr<ui::UiElement>> elements;
int actionModeNum = 0;

const state::WorldActionUiState worldActionUiState;

class SwitchActionListObserver : public ui::UiEventObserver {
  ui::InGameLayout* inGameLayout;

public:
  SwitchActionListObserver(ui::InGameLayout* _inGameLayout)
      : inGameLayout(_inGameLayout) {}
  ~SwitchActionListObserver() override = default;
  void onClick(int x, int y, int button) override {
    actionModeNum++;
    if (actionModeNum >= 3) {
      actionModeNum = 0;
    }
    auto newProps = inGameLayout->getProps();
    if (actionModeNum == 0) {
      LOG(INFO) << "Switching to town mode action list" << LOG_ENDL;
      copyActionTypes(newProps.worldActionTypes, worldActionUiState.townModeActionTypes);
    } else if (actionModeNum == 1) {
      LOG(INFO) << "Switching to town mode fight action list" << LOG_ENDL;
      copyActionTypes(newProps.worldActionTypes, worldActionUiState.townModeFightActionTypes);
    } else if (actionModeNum == 2) {
      LOG(INFO) << "Switching to outdoor mode action list" << LOG_ENDL;
      copyActionTypes(newProps.worldActionTypes, worldActionUiState.outdoorModeActionTypes);
    }
    inGameLayout->setProps(newProps);
  };
};

class SwitchBorderTypeObserver : public ui::UiEventObserver {
  ui::InGameLayout* inGameLayout;

public:
  SwitchBorderTypeObserver(ui::InGameLayout* _inGameLayout)
      : inGameLayout(_inGameLayout) {}
  ~SwitchBorderTypeObserver() override = default;
  void onClick(int x, int y, int button) override {
    auto newProps = inGameLayout->getProps();
    if (newProps.borderType == ui::InGameBorderType::Wide) {
      LOG(INFO) << "Switching to narrow border" << LOG_ENDL;
      newProps.borderType = ui::InGameBorderType::Narrow;
    } else {
      LOG(INFO) << "Switching to wide border" << LOG_ENDL;
      newProps.borderType = ui::InGameBorderType::Wide;
    }
    inGameLayout->setProps(newProps);
  };
};

void initInGameLayoutTest(sdl2w::Window& window) {
  actionModeNum = 0;

  float scale = 1.f;
  auto [windowWidth, windowHeight] = window.getDims();

  auto inGameLayout = new ui::InGameLayout(&window);
  inGameLayout->setPos(0, 0);
  inGameLayout->setScale(scale);
  bmin::DynArray<ui::ChCompactInfoProps> partyMembers;
  {
    ui::ChCompactInfoProps entry;
    entry.characterSpriteName = "actors0_0";
    entry.statusEffectSpriteNames = {
        "ui_status_effect_icons_0",
        "ui_status_effect_icons_1",
    };
    entry.hp = 84;
    entry.mana = 37;
    partyMembers.pushBack(entry);
  }
  {
    ui::ChCompactInfoProps entry;
    entry.characterSpriteName = "actors0_4";
    entry.statusEffectSpriteNames = {
        "ui_status_effect_icons_2",
        "ui_status_effect_icons_3",
        "ui_status_effect_icons_3",
        "ui_status_effect_icons_3",
        "ui_status_effect_icons_3",
    };
    entry.hp = 12;
    entry.mana = 99;
    partyMembers.pushBack(entry);
  }
  {
    ui::ChCompactInfoProps entry;
    entry.characterSpriteName = "actors0_8";
    entry.hp = 200;
    entry.mana = 5;
    partyMembers.pushBack(entry);
  }

  ui::InGameLayoutProps layoutProps;
  layoutProps.width = static_cast<int>(windowWidth / scale);
  layoutProps.height = static_cast<int>(windowHeight / scale);
  copyActionTypes(layoutProps.worldActionTypes, worldActionUiState.townModeActionTypes);
  layoutProps.partyMembers = partyMembers;
  layoutProps.actionButtonScale = 1.5f;
  inGameLayout->setProps(layoutProps);

  auto titleBar = new ui::InGameTitleBar(&window);
  titleBar->setProps(ui::InGameTitleBarProps{
      .title = "Game Title",
      .day = 3,
      .food = 42,
      .ap = 6,
      .showAp = true,
  });
  inGameLayout->setTitleElement(titleBar);

  auto switchActionsButton = new ui::ButtonModal(&window);
  switchActionsButton->setId("switchActionTypes");
  switchActionsButton->setPos(200, 250);
  switchActionsButton->setProps(ui::ButtonModalProps{
      .text = "Switch World Action List",
      .isSelected = false,
      .width = 300,
      .height = 50,
      .fontColor = ui::Colors::White,
  });
  switchActionsButton->addEventObserver(new SwitchActionListObserver(inGameLayout));

  auto switchBorderButton = new ui::ButtonModal(&window);
  switchBorderButton->setId("switchBorderType");
  switchBorderButton->setPos(200, 300);
  switchBorderButton->setProps(ui::ButtonModalProps{
      .text = "Switch Wide / Narrow Border",
      .isSelected = false,
      .width = 300,
      .height = 50,
      .fontColor = ui::Colors::White,
  });
  switchBorderButton->addEventObserver(new SwitchBorderTypeObserver(inGameLayout));

  elements.pushBack(bmin::UniquePtr<ui::UiElement>(switchActionsButton));
  elements.pushBack(bmin::UniquePtr<ui::UiElement>(switchBorderButton));
  elements.pushBack(bmin::UniquePtr<ui::UiElement>(inGameLayout));
}

int main(int argc, char** argv) {
  bmin::String windowTitle = "InGameLayout Test";
  int windowWidth = 640;
  int windowHeight = 480;

  LOG(INFO) << "Start InGameLayout test" << LOG_ENDL;
  srand(time(NULL));

  auto _init = [&](sdl2w::Window& window, sdl2w::Store& store) {
    LOG(INFO) << "InGameLayout test initialized" << LOG_ENDL;
    initInGameLayoutTest(window);
  };

  auto _updateRender = [&](sdl2w::Window& window, sdl2w::Store& store) {
    auto& events = window.getEvents();
    auto mouseX = events.mouseX;
    auto mouseY = events.mouseY;

    for (auto& elem : elements) {
      if (elem) {
        elem->checkHoverEvent(mouseX, mouseY);
      }
    }

    auto& draw = window.getDraw();
    draw.clearScreen();
    for (auto& element : elements) {
      element->render(window.getDeltaTime());
    }
    return true;
  };

  auto _initWithEvents = [&](sdl2w::Window& window, sdl2w::Store& store) {
    _init(window, store);
    auto& events = window.getEvents();
    events.setMouseEvent(sdl2w::MouseEventCb::ON_MOUSE_DOWN,
                         [&](int x, int y, int button) {
                           for (auto& elem : elements) {
                             elem->checkMouseDownEvent(x, y, button);
                           }
                         });
    events.setMouseEvent(sdl2w::MouseEventCb::ON_MOUSE_UP, [&](int x, int y, int button) {
      for (auto& elem : elements) {
        elem->checkMouseUpEvent(x, y, button);
      }
    });
  };

  setupTestUi(argc,
              argv,
              TestUiParams{windowWidth, windowHeight, windowTitle.cStr()},
              _initWithEvents,
              _updateRender,
              [&]() { elements.clear(); });
  LOG(INFO) << "End InGameLayout test" << LOG_ENDL;
  return 0;
}
