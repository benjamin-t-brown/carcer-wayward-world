#include "../../setupTestUi.h"
#include "sdl2w/Draw.h"
#include "sdl2w/Events.h"
#include "sdl2w/Logger.h"
#include "sdl2w/Window.h"
#include "ui/UiElement.h"
#include "ui/elements/buttons/ButtonModal.h"
#include "ui/elements/buttons/ButtonScroll.h"
#include "ui/elements/buttons/ButtonClose.h"
#include "ui/SdlPixels.h" // IWYU pragma: keep
#include <memory>
#include "bmin/String.h"
#include "bmin/DynArray.h"
#include "bmin/UniquePtr.h"

class TestButtonObserver : public ui::UiEventObserver {
  bmin::String id;

public:
  TestButtonObserver(bmin::String _id);
  ~TestButtonObserver() override = default;
  void onMouseDown(int x, int y, int button) override;
  void onMouseUp(int x, int y, int button) override;
  void onClick(int x, int y, int button) override;
};

TestButtonObserver::TestButtonObserver(bmin::String _id) : id(_id) {}

void TestButtonObserver::onMouseDown(int x, int y, int button) {
  LOG(INFO) << "TestButtonObserver mousedown at: " << x << ", " << y
            << " - button: " << button << LOG_ENDL;
}

void TestButtonObserver::onMouseUp(int x, int y, int button) {
  LOG(INFO) << "TestButtonObserver mouseup at: " << x << ", " << y
            << " - button: " << button << LOG_ENDL;
}

void TestButtonObserver::onClick(int x, int y, int button) {
  LOG(INFO) << "TestButtonObserver click at: " << x << ", " << y
            << " - button: " << button << LOG_ENDL;
}

int main(int argc, char** argv) {
  LOG(INFO) << "Start ButtonModal test" << LOG_ENDL;
  srand(time(NULL));

  bmin::DynArray<bmin::UniquePtr<ui::UiElement>> elements;

  auto _init = [&](sdl2w::Window& window, sdl2w::Store& store) {
    LOG(INFO) << "ButtonModal test initialized" << LOG_ENDL;

    // Create first button (not selected)
    auto button1 = bmin::makeUnique<ui::ButtonModal>(&window);
    button1->setId("button1");
    button1->setPos(50, 50);
    button1->setProps(ui::ButtonModalProps{
        .text = "Button 1 (Normal)",
        .isSelected = false,
        .width = 200,
        .height = 50,
    });
    button1->addEventObserver(new TestButtonObserver("button1"));
    elements.pushBack(bmin::UniquePtr<ui::UiElement>(button1.release()));

    // Create second button (selected)
    auto button2 = bmin::makeUnique<ui::ButtonModal>(&window);
    button2->setId("button2");
    button2->setPos(50, 120);
    button2->setProps(ui::ButtonModalProps{
        .text = "Button 2 (Selected)",
        .isSelected = true,
        .width = 200,
        .height = 50,
    });
    button2->addEventObserver(new TestButtonObserver("button2"));
    elements.pushBack(bmin::UniquePtr<ui::UiElement>(button2.release()));

    // Create third button (different size)
    auto button3 = bmin::makeUnique<ui::ButtonModal>(&window);
    button3->setId("button3");
    button3->setPos(50, 190);
    button3->setProps(ui::ButtonModalProps{
        .text = "Wide Button",
        .isSelected = false,
        .width = 300,
        .height = 60,
    });
    button3->addEventObserver(new TestButtonObserver("button3"));
    elements.pushBack(bmin::UniquePtr<ui::UiElement>(button3.release()));

    // Create fourth button (scaled)
    auto button4 = bmin::makeUnique<ui::ButtonModal>(&window);
    button4->setId("button4");
    button4->setPos(50, 270);
    button4->setScale(2.f);
    button4->setProps(ui::ButtonModalProps{
        .text = "Scaled Button",
        .isSelected = true,
        .width = 150,
        .height = 40,
    });
    button4->addEventObserver(new TestButtonObserver("button4"));
    elements.pushBack(bmin::UniquePtr<ui::UiElement>(button4.release()));

    // Create fifth button (empty text)
    auto button5 = bmin::makeUnique<ui::ButtonModal>(&window);
    button5->setId("button5");
    button5->setPos(50, 360);
    button5->setProps(ui::ButtonModalProps{
        .text = "",
        .isSelected = false,
        .width = 200,
        .height = 50,
    });
    button5->addEventObserver(new TestButtonObserver("button5"));
    elements.pushBack(bmin::UniquePtr<ui::UiElement>(button5.release()));

    // Create sixth button (long text)
    auto button6 = bmin::makeUnique<ui::ButtonModal>(&window);
    button6->setId("button6");
    button6->setPos(50, 420);
    button6->setProps(ui::ButtonModalProps{
        .text = "This is selected.",
        .isSelected = true,
        .width = 250,
        .height = 50,
    });
    button6->addEventObserver(new TestButtonObserver("button6"));
    elements.pushBack(bmin::UniquePtr<ui::UiElement>(button6.release()));

    // Create scroll up button
    auto scrollUp = bmin::makeUnique<ui::ButtonScroll>(&window);
    scrollUp->setId("scrollUp");
    scrollUp->setPos(350, 50);
    scrollUp->setProps(ui::ButtonScrollProps{
        .direction = ui::ScrollDirection::UP,
        .isSelected = false,
        .width = 32,
        .height = 32,
    });
    scrollUp->addEventObserver(new TestButtonObserver("scrollUp"));
    elements.pushBack(bmin::UniquePtr<ui::UiElement>(scrollUp.release()));

    // Create scroll down button
    auto scrollDown = bmin::makeUnique<ui::ButtonScroll>(&window);
    scrollDown->setId("scrollDown");
    scrollDown->setPos(350, 100);
    scrollDown->setProps(ui::ButtonScrollProps{
        .direction = ui::ScrollDirection::DOWN,
        .isSelected = true,
        .width = 32,
        .height = 32,
    });
    scrollDown->addEventObserver(new TestButtonObserver("scrollDown"));
    elements.pushBack(bmin::UniquePtr<ui::UiElement>(scrollDown.release()));

    // Create modal close button
    auto modalClose = bmin::makeUnique<ui::ButtonClose>(&window);
    modalClose->setId("modalClose");
    modalClose->setPos(400, 50);
    modalClose->setProps(ui::ButtonCloseProps{
        .closeType = ui::CloseType::MODAL,
    });
    modalClose->addEventObserver(new TestButtonObserver("modalClose"));
    elements.pushBack(bmin::UniquePtr<ui::UiElement>(modalClose.release()));

    {
      auto modalCloseScaled = bmin::makeUnique<ui::ButtonClose>(&window);
      modalCloseScaled->setId("modalCloseScaled");
      modalCloseScaled->setPos(475, 50);
      modalCloseScaled->setScale(2.f);
      modalCloseScaled->setProps(ui::ButtonCloseProps{
          .closeType = ui::CloseType::MODAL,
      });
      modalCloseScaled->addEventObserver(new TestButtonObserver(modalCloseScaled->getId()));
      elements.pushBack(bmin::UniquePtr<ui::UiElement>(modalCloseScaled.release()));
    }

    // Create popup close button
    auto popupClose = bmin::makeUnique<ui::ButtonClose>(&window);
    popupClose->setId("popupClose");
    popupClose->setPos(400, 100);
    popupClose->setProps(ui::ButtonCloseProps{
        .closeType = ui::CloseType::POPUP,
    });
    popupClose->addEventObserver(new TestButtonObserver("popupClose"));
    elements.pushBack(bmin::UniquePtr<ui::UiElement>(popupClose.release()));

    auto& events = window.getEvents();
    events.setMouseEvent(
        //
        sdl2w::MouseEventCb::ON_MOUSE_DOWN,
        [&](int x, int y, int button) {
          LOG(INFO) << "Mouse down at: " << x << ", " << y << " - button: " << button << LOG_ENDL;
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
      argc, argv, TestUiParams{640, 600, "ButtonModal Test"}, _init, _updateRender, [&]() { elements.clear(); });
  LOG(INFO) << "End ButtonModal test" << LOG_ENDL;
  return 0;
}
