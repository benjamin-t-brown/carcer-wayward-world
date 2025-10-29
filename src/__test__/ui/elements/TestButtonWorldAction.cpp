#include "../../setupTestUi.h"
#include "lib/sdl2w/Draw.h"
#include "lib/sdl2w/Events.h"
#include "lib/sdl2w/Logger.h"
#include "lib/sdl2w/Window.h"
#include "types/WorldActions.h"
#include "ui/UiElement.h"
#include "ui/elements/ButtonWorldAction.h"
#include <SDL2/SDL_pixels.h>
#include <memory>

class TestButtonWorldActionObserver : public ui::UiEventObserver {
  std::string id;

public:
  TestButtonWorldActionObserver(std::string _id);
  ~TestButtonWorldActionObserver() override = default;
  void onMouseDown(int x, int y, int button) override;
  void onMouseUp(int x, int y, int button) override;
  void onClick(int x, int y, int button) override;
};

TestButtonWorldActionObserver::TestButtonWorldActionObserver(std::string _id) : id(_id) {}

void TestButtonWorldActionObserver::onMouseDown(int x, int y, int button) {
  LOG(INFO) << "TestButtonWorldActionObserver mousedown at: " << x << ", " << y
            << " - button: " << button << LOG_ENDL;
}

void TestButtonWorldActionObserver::onMouseUp(int x, int y, int button) {
  LOG(INFO) << "TestButtonWorldActionObserver mouseup at: " << x << ", " << y
            << " - button: " << button << LOG_ENDL;
}

void TestButtonWorldActionObserver::onClick(int x, int y, int button) {
  LOG(INFO) << "TestButtonWorldActionObserver click at: " << x << ", " << y
            << " - button: " << button << LOG_ENDL;
}

int main(int argc, char** argv) {
  LOG(INFO) << "Start ButtonWorldAction test" << LOG_ENDL;
  sdl2w::Window::init();
  srand(time(NULL));

  std::vector<std::unique_ptr<ui::UiElement>> elements;

  auto _init = [&](sdl2w::Window& window, sdl2w::Store& store) {
    LOG(INFO) << "ButtonWorldAction test initialized" << LOG_ENDL;

    // Create EXAMINE button
    auto examineButton = std::make_unique<ui::ButtonWorldAction>(&window);
    examineButton->setId("examineButton");
    ui::BaseStyle examineStyle;
    examineStyle.x = 50;
    examineStyle.y = 50;
    examineButton->setStyle(examineStyle);
    ui::ButtonWorldActionProps examineProps;
    examineProps.worldActionType = types::WorldActionType::EXAMINE;
    examineButton->setProps(examineProps);
    examineButton->addEventObserver(
        std::make_unique<TestButtonWorldActionObserver>("examineButton"));
    elements.push_back(std::move(examineButton));

    // Create GET button
    auto getButton = std::make_unique<ui::ButtonWorldAction>(&window);
    getButton->setId("getButton");
    ui::BaseStyle getStyle;
    getStyle.x = 200;
    getStyle.y = 50;
    getButton->setStyle(getStyle);
    ui::ButtonWorldActionProps getProps;
    getProps.worldActionType = types::WorldActionType::GET;
    getButton->setProps(getProps);
    getButton->addEventObserver(
        std::make_unique<TestButtonWorldActionObserver>("getButton"));
    elements.push_back(std::move(getButton));

    // Create SNEAK button
    auto sneakButton = std::make_unique<ui::ButtonWorldAction>(&window);
    sneakButton->setId("sneakButton");
    ui::BaseStyle sneakStyle;
    sneakStyle.x = 350;
    sneakStyle.y = 50;
    sneakButton->setStyle(sneakStyle);
    ui::ButtonWorldActionProps sneakProps;
    sneakProps.worldActionType = types::WorldActionType::SNEAK;
    sneakButton->setProps(sneakProps);
    sneakButton->addEventObserver(
        std::make_unique<TestButtonWorldActionObserver>("sneakButton"));
    elements.push_back(std::move(sneakButton));

    // Create TALK button
    auto talkButton = std::make_unique<ui::ButtonWorldAction>(&window);
    talkButton->setId("talkButton");
    ui::BaseStyle talkStyle;
    talkStyle.x = 50;
    talkStyle.y = 120;
    talkButton->setStyle(talkStyle);
    ui::ButtonWorldActionProps talkProps;
    talkProps.worldActionType = types::WorldActionType::TALK;
    talkButton->setProps(talkProps);
    talkButton->addEventObserver(
        std::make_unique<TestButtonWorldActionObserver>("talkButton"));
    elements.push_back(std::move(talkButton));

    // Create JUMP button
    auto jumpButton = std::make_unique<ui::ButtonWorldAction>(&window);
    jumpButton->setId("jumpButton");
    ui::BaseStyle jumpStyle;
    jumpStyle.x = 200;
    jumpStyle.y = 120;
    jumpButton->setStyle(jumpStyle);
    ui::ButtonWorldActionProps jumpProps;
    jumpProps.worldActionType = types::WorldActionType::JUMP;
    jumpButton->setProps(jumpProps);
    jumpButton->addEventObserver(
        std::make_unique<TestButtonWorldActionObserver>("jumpButton"));
    elements.push_back(std::move(jumpButton));

    // Create ABILITY button
    auto abilityButton = std::make_unique<ui::ButtonWorldAction>(&window);
    abilityButton->setId("abilityButton");
    ui::BaseStyle abilityStyle;
    abilityStyle.x = 350;
    abilityStyle.y = 120;
    abilityButton->setStyle(abilityStyle);
    ui::ButtonWorldActionProps abilityProps;
    abilityProps.worldActionType = types::WorldActionType::ABILITY;
    abilityButton->setProps(abilityProps);
    abilityButton->addEventObserver(
        std::make_unique<TestButtonWorldActionObserver>("abilityButton"));
    elements.push_back(std::move(abilityButton));

    // Create FIGHT button
    auto fightButton = std::make_unique<ui::ButtonWorldAction>(&window);
    fightButton->setId("fightButton");
    ui::BaseStyle fightStyle;
    fightStyle.x = 50;
    fightStyle.y = 190;
    fightButton->setStyle(fightStyle);
    ui::ButtonWorldActionProps fightProps;
    fightProps.worldActionType = types::WorldActionType::START_FIGHT;
    fightButton->setProps(fightProps);
    fightButton->addEventObserver(
        std::make_unique<TestButtonWorldActionObserver>("fightButton"));
    elements.push_back(std::move(fightButton));

    // // Create SHOOT button
    auto shootButton = std::make_unique<ui::ButtonWorldAction>(&window);
    shootButton->setId("shootButton");
    ui::BaseStyle shootStyle;
    shootStyle.x = 200;
    shootStyle.y = 190;
    shootButton->setStyle(shootStyle);
    ui::ButtonWorldActionProps shootProps;
    shootProps.worldActionType = types::WorldActionType::SHOOT;
    shootButton->setProps(shootProps);
    shootButton->addEventObserver(
        std::make_unique<TestButtonWorldActionObserver>("shootButton"));
    elements.push_back(std::move(shootButton));

    // Create INVENTORY button
    auto inventoryButton = std::make_unique<ui::ButtonWorldAction>(&window);
    inventoryButton->setId("inventoryButton");
    ui::BaseStyle inventoryStyle;
    inventoryStyle.x = 350;
    inventoryStyle.y = 190;
    inventoryStyle.scale = 1.5f;
    inventoryButton->setStyle(inventoryStyle);
    ui::ButtonWorldActionProps inventoryProps;
    inventoryProps.worldActionType = types::WorldActionType::INVENTORY;
    inventoryButton->setProps(inventoryProps);
    inventoryButton->addEventObserver(
        std::make_unique<TestButtonWorldActionObserver>("inventoryButton"));
    elements.push_back(std::move(inventoryButton));

    auto& events = window.getEvents();
    events.setMouseEvent(
        //
        sdl2w::MouseEventCb::ON_MOUSE_DOWN,
        [&](int x, int y, int button) {
          LOG(INFO) << "Mouse down at: " << x << ", " << y << " - button: " << button
                    << LOG_ENDL;
          for (auto& elem : elements) {
            elem->checkMouseDownEvent(x, y, button);
          }
        });
    events.setMouseEvent(
        //
        sdl2w::MouseEventCb::ON_MOUSE_UP,
        [&](int x, int y, int button) {
          for (auto& elem : elements) {
            elem->checkMouseUpEvent(x, y, button);
          }
        });
  };

  auto _updateRender = [&](sdl2w::Window& window, sdl2w::Store& store) {
    // Get mouse position for hover events
    auto& events = window.getEvents();
    auto mouseX = events.mouseX;
    auto mouseY = events.mouseY;

    // Check hover events for all buttons
    for (auto& elem : elements) {
      if (elem) {
        elem->checkHoverEvent(mouseX, mouseY);
        elem->render(window.getDeltaTime());
      }
    }

    return true;
  };

  setupTestUi(
      argc, argv, TestUiParams{640, 400, "ButtonWorldAction Test"}, _init, _updateRender);
  LOG(INFO) << "End ButtonWorldAction test" << LOG_ENDL;
  sdl2w::Window::unInit();
  return 0;
}
