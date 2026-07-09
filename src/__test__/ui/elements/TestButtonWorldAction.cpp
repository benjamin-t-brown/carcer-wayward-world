#include "../../setupTestUi.h"
#include "bmin/DynArray.h"
#include "bmin/String.h"
#include "bmin/UniquePtr.h"
#include "lib/sdl2w/Draw.h"
#include "lib/sdl2w/Events.h"
#include "lib/sdl2w/Logger.h"
#include "lib/sdl2w/Window.h"
#include "state/WorldActions.h"
#include "ui/UiElement.h"
#include "ui/elements/buttons/ButtonWorldAction.h"
#include "ui/SdlPixels.h" // IWYU pragma: keep
#include <memory>

class TestButtonWorldActionObserver : public ui::UiEventObserver {
  bmin::String id;

public:
  TestButtonWorldActionObserver(bmin::String _id);
  ~TestButtonWorldActionObserver() override = default;
  void onMouseDown(int x, int y, int button) override;
  void onMouseUp(int x, int y, int button) override;
  void onClick(int x, int y, int button) override;
};

TestButtonWorldActionObserver::TestButtonWorldActionObserver(bmin::String _id) : id(_id) {}

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
  srand(time(NULL));

  bmin::DynArray<bmin::UniquePtr<ui::UiElement>> elements;

  auto _init = [&](sdl2w::Window& window, sdl2w::Store& store) {
    LOG(INFO) << "ButtonWorldAction test initialized" << LOG_ENDL;

    // Create EXAMINE button
    auto examineButton = bmin::makeUnique<ui::ButtonWorldAction>(&window);
    examineButton->setId("examineButton");
    ui::BaseStyle examineStyle;
    examineStyle.x = 50;
    examineStyle.y = 50;
    examineButton->setStyle(examineStyle);
    ui::ButtonWorldActionProps examineProps;
    examineProps.worldActionType = state::WorldActionType::EXAMINE;
    examineButton->setProps(examineProps);
    examineButton->addEventObserver(new TestButtonWorldActionObserver("examineButton"));
    elements.pushBack(bmin::UniquePtr<ui::UiElement>(examineButton.release()));

    // Create GET button
    auto getButton = bmin::makeUnique<ui::ButtonWorldAction>(&window);
    getButton->setId("getButton");
    ui::BaseStyle getStyle;
    getStyle.x = 200;
    getStyle.y = 50;
    getButton->setStyle(getStyle);
    ui::ButtonWorldActionProps getProps;
    getProps.worldActionType = state::WorldActionType::GET;
    getButton->setProps(getProps);
    getButton->addEventObserver(new TestButtonWorldActionObserver("getButton"));
    elements.pushBack(bmin::UniquePtr<ui::UiElement>(getButton.release()));

    // Create SNEAK button
    auto sneakButton = bmin::makeUnique<ui::ButtonWorldAction>(&window);
    sneakButton->setId("sneakButton");
    ui::BaseStyle sneakStyle;
    sneakStyle.x = 350;
    sneakStyle.y = 50;
    sneakButton->setStyle(sneakStyle);
    ui::ButtonWorldActionProps sneakProps;
    sneakProps.worldActionType = state::WorldActionType::SNEAK;
    sneakButton->setProps(sneakProps);
    sneakButton->addEventObserver(new TestButtonWorldActionObserver("sneakButton"));
    elements.pushBack(bmin::UniquePtr<ui::UiElement>(sneakButton.release()));

    // Create TALK button
    auto talkButton = bmin::makeUnique<ui::ButtonWorldAction>(&window);
    talkButton->setId("talkButton");
    ui::BaseStyle talkStyle;
    talkStyle.x = 50;
    talkStyle.y = 120;
    talkButton->setStyle(talkStyle);
    ui::ButtonWorldActionProps talkProps;
    talkProps.worldActionType = state::WorldActionType::TALK;
    talkButton->setProps(talkProps);
    talkButton->addEventObserver(new TestButtonWorldActionObserver("talkButton"));
    elements.pushBack(bmin::UniquePtr<ui::UiElement>(talkButton.release()));

    // Create JUMP button
    auto jumpButton = bmin::makeUnique<ui::ButtonWorldAction>(&window);
    jumpButton->setId("jumpButton");
    ui::BaseStyle jumpStyle;
    jumpStyle.x = 200;
    jumpStyle.y = 120;
    jumpButton->setStyle(jumpStyle);
    ui::ButtonWorldActionProps jumpProps;
    jumpProps.worldActionType = state::WorldActionType::JUMP;
    jumpButton->setProps(jumpProps);
    jumpButton->addEventObserver(new TestButtonWorldActionObserver("jumpButton"));
    elements.pushBack(bmin::UniquePtr<ui::UiElement>(jumpButton.release()));

    // Create ABILITY button
    auto abilityButton = bmin::makeUnique<ui::ButtonWorldAction>(&window);
    abilityButton->setId("abilityButton");
    ui::BaseStyle abilityStyle;
    abilityStyle.x = 350;
    abilityStyle.y = 120;
    abilityButton->setStyle(abilityStyle);
    ui::ButtonWorldActionProps abilityProps;
    abilityProps.worldActionType = state::WorldActionType::ABILITY;
    abilityButton->setProps(abilityProps);
    abilityButton->addEventObserver(new TestButtonWorldActionObserver("abilityButton"));
    elements.pushBack(bmin::UniquePtr<ui::UiElement>(abilityButton.release()));

    // Create FIGHT button
    auto fightButton = bmin::makeUnique<ui::ButtonWorldAction>(&window);
    fightButton->setId("fightButton");
    ui::BaseStyle fightStyle;
    fightStyle.x = 50;
    fightStyle.y = 190;
    fightButton->setStyle(fightStyle);
    ui::ButtonWorldActionProps fightProps;
    fightProps.worldActionType = state::WorldActionType::START_FIGHT;
    fightButton->setProps(fightProps);
    fightButton->addEventObserver(new TestButtonWorldActionObserver("fightButton"));
    elements.pushBack(bmin::UniquePtr<ui::UiElement>(fightButton.release()));

    // // Create SHOOT button
    auto shootButton = bmin::makeUnique<ui::ButtonWorldAction>(&window);
    shootButton->setId("shootButton");
    ui::BaseStyle shootStyle;
    shootStyle.x = 200;
    shootStyle.y = 190;
    shootButton->setStyle(shootStyle);
    ui::ButtonWorldActionProps shootProps;
    shootProps.worldActionType = state::WorldActionType::SHOOT;
    shootButton->setProps(shootProps);
    shootButton->addEventObserver(new TestButtonWorldActionObserver("shootButton"));
    elements.pushBack(bmin::UniquePtr<ui::UiElement>(shootButton.release()));

    // Create INVENTORY button
    auto inventoryButton = bmin::makeUnique<ui::ButtonWorldAction>(&window);
    inventoryButton->setId("inventoryButton");
    ui::BaseStyle inventoryStyle;
    inventoryStyle.x = 350;
    inventoryStyle.y = 190;
    inventoryStyle.scale = 1.5f;
    inventoryButton->setStyle(inventoryStyle);
    ui::ButtonWorldActionProps inventoryProps;
    inventoryProps.worldActionType = state::WorldActionType::INVENTORY;
    inventoryButton->setProps(inventoryProps);
    inventoryButton->addEventObserver(
        new TestButtonWorldActionObserver("inventoryButton"));
    elements.pushBack(bmin::UniquePtr<ui::UiElement>(inventoryButton.release()));

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
      argc, argv, TestUiParams{640, 400, "ButtonWorldAction Test"}, _init, _updateRender, [&]() { elements.clear(); });
  LOG(INFO) << "End ButtonWorldAction test" << LOG_ENDL;
  return 0;
}
