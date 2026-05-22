#include "../../setupTestUi.h"
#include "lib/sdl2w/Draw.h"
#include "lib/sdl2w/Logger.h"
#include "lib/sdl2w/Window.h"
#include "model/WorldActions.h"
#include "ui/SdlPixels.h" // IWYU pragma: keep
#include "ui/UiElement.h"
#include "ui/elements/TextLine.h"
#include "ui/elements/buttons/ButtonModal.h"
#include "ui/layouts/InGameLayout.h"
#include <memory>
#include <string>

std::vector<std::unique_ptr<ui::UiElement>> elements;
int actionModeNum = 0;

const std::vector<model::WorldActionType> townModeActionTypes = {
    model::WorldActionType::START_FIGHT,
    model::WorldActionType::EXAMINE,
    model::WorldActionType::TALK,
    model::WorldActionType::ABILITY,
    model::WorldActionType::SNEAK,
    model::WorldActionType::JUMP,
    model::WorldActionType::SHOOT,
    model::WorldActionType::INTERACT,
    model::WorldActionType::GET,
    model::WorldActionType::JOURNAL,
    model::WorldActionType::INVENTORY,
    model::WorldActionType::STATUS,
    model::WorldActionType::MAP,
};

const std::vector<model::WorldActionType> townModeFightActionTypes = {
    model::WorldActionType::END_FIGHT,
    model::WorldActionType::EXAMINE,
    model::WorldActionType::ABILITY,
    model::WorldActionType::JUMP,
    model::WorldActionType::SHOOT,
    model::WorldActionType::DEFEND,
    model::WorldActionType::GET,
    model::WorldActionType::INVENTORY,
    model::WorldActionType::STATUS,
};

const std::vector<model::WorldActionType> outdoorModeActionTypes = {
    model::WorldActionType::EXAMINE,
    model::WorldActionType::ABILITY,
    model::WorldActionType::MAP_OUTDOOR,
    model::WorldActionType::REST,
    model::WorldActionType::INVENTORY,
    model::WorldActionType::STATUS,
    model::WorldActionType::JOURNAL,
};

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
      newProps.worldActionTypes = townModeActionTypes;
    } else if (actionModeNum == 1) {
      LOG(INFO) << "Switching to town mode fight action list" << LOG_ENDL;
      newProps.worldActionTypes = townModeFightActionTypes;
    } else if (actionModeNum == 2) {
      LOG(INFO) << "Switching to outdoor mode action list" << LOG_ENDL;
      newProps.worldActionTypes = outdoorModeActionTypes;
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

void initInGameLayoutTest(sdl2w::Window& window, ui::InGameBorderType borderType) {
  actionModeNum = 0;

  float scale = 2.f;
  auto [windowWidth, windowHeight] = window.getDims();

  auto inGameLayout = new ui::InGameLayout(&window);
  auto& layoutStyle = inGameLayout->getStyle();
  layoutStyle.width = windowWidth / scale;
  layoutStyle.height = windowHeight / scale;
  layoutStyle.x = 0;
  layoutStyle.y = 0;
  layoutStyle.scale = scale;
  inGameLayout->setProps(ui::InGameLayoutProps{
      .worldActionTypes = townModeActionTypes,
      .actionButtonScale = 1.f,
      .borderType = borderType,
  });

  auto titleElement = new ui::TextLine(&window);
  auto& titleStyle = titleElement->getStyle();
  setBaseFontConfig(titleStyle, ui::BaseFontConfig::MODAL_TITLE);
  titleStyle.fontColor = ui::Colors::White;
  ui::TextLineProps titleProps;
  ui::TextBlock titleBlock;
  titleBlock.text = "Game Title";
  titleProps.textBlocks.push_back(titleBlock);
  titleElement->setProps(titleProps);
  inGameLayout->setTitleElement(titleElement);

  auto switchActionsButton = new ui::ButtonModal(&window);
  switchActionsButton->setId("switchActionTypes");
  auto& switchActionsStyle = switchActionsButton->getStyle();
  switchActionsStyle.x = 200;
  switchActionsStyle.y = 200;
  switchActionsStyle.width = 300;
  switchActionsStyle.height = 50;
  switchActionsStyle.fontColor = ui::Colors::White;
  ui::ButtonModalProps switchActionsProps;
  switchActionsProps.text = "Switch World Action List";
  switchActionsProps.isSelected = false;
  switchActionsButton->setProps(switchActionsProps);
  switchActionsButton->addEventObserver(new SwitchActionListObserver(inGameLayout));

  auto switchBorderButton = new ui::ButtonModal(&window);
  switchBorderButton->setId("switchBorderType");
  auto& switchBorderStyle = switchBorderButton->getStyle();
  switchBorderStyle.x = 200;
  switchBorderStyle.y = 260;
  switchBorderStyle.width = 300;
  switchBorderStyle.height = 50;
  switchBorderStyle.fontColor = ui::Colors::White;
  ui::ButtonModalProps switchBorderProps;
  switchBorderProps.text = "Switch Wide / Narrow Border";
  switchBorderProps.isSelected = false;
  switchBorderButton->setProps(switchBorderProps);
  switchBorderButton->addEventObserver(new SwitchBorderTypeObserver(inGameLayout));

  elements.push_back(std::unique_ptr<ui::UiElement>(switchActionsButton));
  elements.push_back(std::unique_ptr<ui::UiElement>(switchBorderButton));
  elements.push_back(std::unique_ptr<ui::UiElement>(inGameLayout));
}

int main(int argc, char** argv) {
  ui::InGameBorderType borderType = ui::InGameBorderType::Wide;
  std::string windowTitle = "InGameLayout Test";
  int windowWidth = 800;
  int windowHeight = 800;

  borderType = ui::InGameBorderType::Narrow;

  LOG(INFO) << "Start InGameLayout test" << LOG_ENDL;
  srand(time(NULL));

  auto _init = [&](sdl2w::Window& window, sdl2w::Store& store) {
    LOG(INFO) << "InGameLayout test initialized" << LOG_ENDL;
    initInGameLayoutTest(window, borderType);
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
              TestUiParams{windowWidth, windowHeight, windowTitle.c_str()},
              _initWithEvents,
              _updateRender,
              [&]() { elements.clear(); });
  LOG(INFO) << "End InGameLayout test" << LOG_ENDL;
  return 0;
}
