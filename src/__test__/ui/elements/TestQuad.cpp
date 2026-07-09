#include "sdl2w/Draw.h"
#include "sdl2w/Logger.h"
#include "sdl2w/Window.h"
#include "bmin/DynArray.h"
#include "bmin/UniquePtr.h"

#include "../../setupTestUi.h"
#include "ui/SdlPixels.h" // IWYU pragma: keep
#include "ui/UiElement.h"
#include "ui/colors.h"
#include "ui/elements/Quad.h"
#include <memory>

ui::Quad* createBasicQuad(sdl2w::Window* window, int w, int h) {
  auto q = new ui::Quad(window);
  q->setPos(0, 0);
  q->setProps(ui::QuadProps{
      .width = w,
      .height = h,
  });
  return q;
}

int main(int argc, char** argv) {
  LOG(INFO) << "Start test" << LOG_ENDL;
  srand(time(NULL));

  bmin::DynArray<bmin::UniquePtr<ui::UiElement>> elements;

  auto _init = [&](sdl2w::Window& window, sdl2w::Store& store) {
    {
      auto q = createBasicQuad(&window, 40, 40);
      q->setPos(10, 10);
      q->setProps({
          .width = 40,
          .height = 40,
          .bgColor = ui::Colors::DarkGrey,
          .bgSprite = "actors0_0",
      });
      elements.pushBack(bmin::UniquePtr<ui::UiElement>(q));
    }

    {
      auto q = createBasicQuad(&window, 40, 40);
      q->setPos(52, 10);
      q->setScale(2.0f);
      q->setProps({
          .width = 40,
          .height = 40,
          .bgColor = ui::Colors::DarkGrey,
          .bgSprite = "actors0_0",
      });
      elements.pushBack(bmin::UniquePtr<ui::UiElement>(q));
    }

    {
      auto q = createBasicQuad(&window, 40, 40);
      q->setPos(52 + 40 * 2, 10);
      q->setScale(4.0f);
      q->setProps({
          .width = 40,
          .height = 40,
          .bgColor = ui::Colors::DarkGrey,
          .bgSprite = "actors0_0",
      });
      elements.pushBack(bmin::UniquePtr<ui::UiElement>(q));
    }

    // check text and border ----

    {
      auto q = createBasicQuad(&window, 40, 40);
      q->setPos(10, 150);
      q->setProps({
          .width = 40,
          .height = 40,
          .bgColor = ui::Colors::DarkBlue,
          .bgSprite = "actors0_2",
          .borderColor = ui::Colors::Red,
          .borderSize = 4,
      });
      elements.pushBack(bmin::UniquePtr<ui::UiElement>(q));
    }

    {
      auto q = createBasicQuad(&window, 40, 40);
      q->setPos(52, 150);
      q->setScale(2.0f);
      q->setProps({
          .width = 40,
          .height = 40,
          .bgColor = ui::Colors::DarkBlue,
          .bgSprite = "actors0_2",
          .borderColor = ui::Colors::Red,
          .borderSize = 4,
      });
      elements.pushBack(bmin::UniquePtr<ui::UiElement>(q));
    }

    {
      auto q = createBasicQuad(&window, 40, 40);
      q->setPos(52 + 40 * 2, 150);
      q->setScale(4.0f);
      q->setProps({
          .width = 40,
          .height = 40,
          .bgColor = ui::Colors::DarkBlue,
          .bgSprite = "actors0_2",
          .borderColor = ui::Colors::Red,
          .borderSize = 4,
      });
      elements.pushBack(bmin::UniquePtr<ui::UiElement>(q));
    }

    // Test quad in quad
    {
      auto q = createBasicQuad(&window, 100, 100);
      q->setPos(330, 10);
      q->setProps({
          .width = 100,
          .height = 100,
          .bgColor = ui::Colors::DarkBlue,
          // .bgSprite = "actors0_2",
          .borderColor = ui::Colors::Red,
          .borderSize = 1,
      });
      elements.pushBack(bmin::UniquePtr<ui::UiElement>(q));

      auto q2 = createBasicQuad(&window, 75, 75);
      q2->setPos(10, 10);
      q2->setProps({
          .width = 75,
          .height = 75,
          .bgColor = ui::Colors::DarkGrey,
          .bgSprite = "actors0_0",
          .borderColor = ui::Colors::Transparent,
          .borderSize = 0,
      });
      q->addChild(q2);
    }

    {
      auto q = createBasicQuad(&window, 100, 100);
      q->setPos(330, 100);
      q->setScale(3.0f);
      q->setProps({
          .width = 100,
          .height = 100,
          .bgColor = ui::Colors::DarkBlue,
          // .bgSprite = "actors0_2",
          .borderColor = ui::Colors::Red,
          .borderSize = 1,
      });
      elements.pushBack(bmin::UniquePtr<ui::UiElement>(q));

      auto q2 = createBasicQuad(&window, 75, 75);
      q2->setPos(10, 10);
      q2->setProps({
          .width = 75,
          .height = 75,
          .bgColor = ui::Colors::DarkGrey,
          .bgSprite = "actors0_0",
          .borderColor = ui::Colors::Transparent,
          .borderSize = 0,
      });
      q->addChild(q2);
    }
  };

  auto _updateRender = [&](sdl2w::Window& window, sdl2w::Store& store) {
    for (auto& elem : elements) {
      if (elem) {
        elem->render(window.getDeltaTime());
      }
    }
    return true;
  };

  setupTestUi(argc, argv, TestUiParams{640, 480}, _init, _updateRender, [&]() {
    elements.clear();
  });
  LOG(INFO) << "End test" << LOG_ENDL;
  return 0;
}
