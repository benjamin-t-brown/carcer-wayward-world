#include "../../setupTestUi.h"
#include "sdl2w/Draw.h"
#include "sdl2w/Events.h"
#include "sdl2w/Logger.h"
#include "sdl2w/Window.h"
#include "ui/UiElement.h"
#include "ui/colors.h"
#include "ui/elements/buttons/ButtonTextWrap.h"
#include "ui/SdlPixels.h" // IWYU pragma: keep
#include <memory>
#include "bmin/String.h"
#include "bmin/DynArray.h"
#include "bmin/UniquePtr.h"

class TestButtonTextWrapObserver : public ui::UiEventObserver {
  bmin::String id;

public:
  TestButtonTextWrapObserver(bmin::String _id);
  ~TestButtonTextWrapObserver() override = default;
  void onMouseDown(int x, int y, int button) override;
  void onMouseUp(int x, int y, int button) override;
  void onClick(int x, int y, int button) override;
};

TestButtonTextWrapObserver::TestButtonTextWrapObserver(bmin::String _id) : id(_id) {}

void TestButtonTextWrapObserver::onMouseDown(int x, int y, int button) {
  LOG(INFO) << "TestButtonTextWrapObserver mousedown at: " << x << ", " << y
            << " - button: " << button << LOG_ENDL;
}

void TestButtonTextWrapObserver::onMouseUp(int x, int y, int button) {
  LOG(INFO) << "TestButtonTextWrapObserver mouseup at: " << x << ", " << y
            << " - button: " << button << LOG_ENDL;
}

void TestButtonTextWrapObserver::onClick(int x, int y, int button) {
  LOG(INFO) << "TestButtonTextWrapObserver click at: " << x << ", " << y
            << " - button: " << button << LOG_ENDL;
}

int main(int argc, char** argv) {
  LOG(INFO) << "Start ButtonTextWrap test" << LOG_ENDL;
  srand(time(NULL));

  bmin::DynArray<bmin::UniquePtr<ui::UiElement>> elements;

  auto _init = [&](sdl2w::Window& window, sdl2w::Store& store) {
    LOG(INFO) << "ButtonTextWrap test initialized" << LOG_ENDL;

    // Create first button with short text
    auto button1 = bmin::makeUnique<ui::ButtonTextWrap>(&window);
    button1->setId("button1");
    button1->setPos(50, 50);
    button1->setProps(ui::ButtonTextWrapProps{
        .text = "Short Text",
        .width = 200,
        .isSelected = false,
    });
    button1->addEventObserver(new TestButtonTextWrapObserver("button1"));
    elements.pushBack(bmin::UniquePtr<ui::UiElement>(button1.release()));

    // Create second button with long text that wraps
    auto button2 = bmin::makeUnique<ui::ButtonTextWrap>(&window);
    button2->setId("button2");
    button2->setPos(50, 130);
    button2->setProps(ui::ButtonTextWrapProps{
        .text = "This is a much longer text that should wrap to multiple lines "
                "when it exceeds the button width. Lol wow that's amazing bro.",
        .width = 300,
        .verticalPadding = 10,
        .horizontalPadding = 10,
        .isSelected = true,
        .fontColor = ui::Colors::Black,
    });
    button2->addEventObserver(new TestButtonTextWrapObserver("button2"));
    elements.pushBack(bmin::UniquePtr<ui::UiElement>(button2.release()));

    // Create third button with medium text
    auto button3 = bmin::makeUnique<ui::ButtonTextWrap>(&window);
    button3->setId("button3");
    button3->setPos(50, 230);
    button3->setProps(ui::ButtonTextWrapProps{
        .text = "Medium length text that might wrap",
        .width = 250,
        .isSelected = false,
        .fontColor = ui::Colors::Black,
    });
    button3->addEventObserver(new TestButtonTextWrapObserver("button3"));
    elements.pushBack(bmin::UniquePtr<ui::UiElement>(button3.release()));

    // Create fourth button with very long text
    auto button4 = bmin::makeUnique<ui::ButtonTextWrap>(&window);
    button4->setId("button4");
    button4->setPos(50, 320);
    button4->setProps(ui::ButtonTextWrapProps{
        .text = "This is an extremely long text that will definitely wrap to "
                "multiple lines and should demonstrate the text wrapping "
                "functionality of the ButtonTextWrap element very well",
        .width = 400,
        .isSelected = false,
    });
    button4->addEventObserver(new TestButtonTextWrapObserver("button4"));
    elements.pushBack(bmin::UniquePtr<ui::UiElement>(button4.release()));

    // Create fifth button with single line text
    auto button5 = bmin::makeUnique<ui::ButtonTextWrap>(&window);
    button5->setId("button5");
    button5->setPos(50, 440);
    button5->setScale(2.f);
    button5->setProps(ui::ButtonTextWrapProps{
        .text = "Scaled Button",
        .width = 150,
        .isSelected = true,
    });
    button5->addEventObserver(new TestButtonTextWrapObserver("button5"));
    elements.pushBack(bmin::UniquePtr<ui::UiElement>(button5.release()));

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
    window.getDraw().setBackgroundColor({200, 200, 200});

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
      argc, argv, TestUiParams{600, 600, "ButtonTextWrap Test"}, _init, _updateRender, [&]() { elements.clear(); });
  LOG(INFO) << "End ButtonTextWrap test" << LOG_ENDL;
  return 0;
}
