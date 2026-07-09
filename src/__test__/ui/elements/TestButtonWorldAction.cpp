#include "../../setupTestUi.h"
#include "bmin/DynArray.h"
#include "bmin/String.h"
#include "bmin/UniquePtr.h"
#include "sdl2w/Draw.h"
#include "sdl2w/Events.h"
#include "sdl2w/Logger.h"
#include "sdl2w/Window.h"
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
    examineButton->setPos(50, 50);
    examineButton->setProps(ui::ButtonWorldActionProps{
        .worldActionType = state::WorldActionType::EXAMINE,
    });
    examineButton->addEventObserver(new TestButtonWorldActionObserver("examineButton"));
    elements.pushBack(bmin::UniquePtr<ui::UiElement>(examineButton.release()));

    // Create GET button
    auto getButton = bmin::makeUnique<ui::ButtonWorldAction>(&window);
    getButton->setId("getButton");
    getButton->setPos(200, 50);
    getButton->setProps(ui::ButtonWorldActionProps{
        .worldActionType = state::WorldActionType::GET,
    });
    getButton->addEventObserver(new TestButtonWorldActionObserver("getButton"));
    elements.pushBack(bmin::UniquePtr<ui::UiElement>(getButton.release()));

    // Create SNEAK button
    auto sneakButton = bmin::makeUnique<ui::ButtonWorldAction>(&window);
    sneakButton->setId("sneakButton");
    sneakButton->setPos(350, 50);
    sneakButton->setProps(ui::ButtonWorldActionProps{
        .worldActionType = state::WorldActionType::SNEAK,
    });
    sneakButton->addEventObserver(new TestButtonWorldActionObserver("sneakButton"));
    elements.pushBack(bmin::UniquePtr<ui::UiElement>(sneakButton.release()));

    // Create TALK button
    auto talkButton = bmin::makeUnique<ui::ButtonWorldAction>(&window);
    talkButton->setId("talkButton");
    talkButton->setPos(50, 120);
    talkButton->setProps(ui::ButtonWorldActionProps{
        .worldActionType = state::WorldActionType::TALK,
    });
    talkButton->addEventObserver(new TestButtonWorldActionObserver("talkButton"));
    elements.pushBack(bmin::UniquePtr<ui::UiElement>(talkButton.release()));

    // Create JUMP button
    auto jumpButton = bmin::makeUnique<ui::ButtonWorldAction>(&window);
    jumpButton->setId("jumpButton");
    jumpButton->setPos(200, 120);
    jumpButton->setProps(ui::ButtonWorldActionProps{
        .worldActionType = state::WorldActionType::JUMP,
    });
    jumpButton->addEventObserver(new TestButtonWorldActionObserver("jumpButton"));
    elements.pushBack(bmin::UniquePtr<ui::UiElement>(jumpButton.release()));

    // Create ABILITY button
    auto abilityButton = bmin::makeUnique<ui::ButtonWorldAction>(&window);
    abilityButton->setId("abilityButton");
    abilityButton->setPos(350, 120);
    abilityButton->setProps(ui::ButtonWorldActionProps{
        .worldActionType = state::WorldActionType::ABILITY,
    });
    abilityButton->addEventObserver(new TestButtonWorldActionObserver("abilityButton"));
    elements.pushBack(bmin::UniquePtr<ui::UiElement>(abilityButton.release()));

    // Create FIGHT button
    auto fightButton = bmin::makeUnique<ui::ButtonWorldAction>(&window);
    fightButton->setId("fightButton");
    fightButton->setPos(50, 190);
    fightButton->setProps(ui::ButtonWorldActionProps{
        .worldActionType = state::WorldActionType::START_FIGHT,
    });
    fightButton->addEventObserver(new TestButtonWorldActionObserver("fightButton"));
    elements.pushBack(bmin::UniquePtr<ui::UiElement>(fightButton.release()));

    // Create SHOOT button
    auto shootButton = bmin::makeUnique<ui::ButtonWorldAction>(&window);
    shootButton->setId("shootButton");
    shootButton->setPos(200, 190);
    shootButton->setProps(ui::ButtonWorldActionProps{
        .worldActionType = state::WorldActionType::SHOOT,
    });
    shootButton->addEventObserver(new TestButtonWorldActionObserver("shootButton"));
    elements.pushBack(bmin::UniquePtr<ui::UiElement>(shootButton.release()));

    // Create INVENTORY button
    auto inventoryButton = bmin::makeUnique<ui::ButtonWorldAction>(&window);
    inventoryButton->setId("inventoryButton");
    inventoryButton->setPos(350, 190);
    inventoryButton->setScale(1.5f);
    inventoryButton->setProps(ui::ButtonWorldActionProps{
        .worldActionType = state::WorldActionType::INVENTORY,
    });
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
