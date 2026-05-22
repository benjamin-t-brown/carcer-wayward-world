#include "lib/sdl2w/Draw.h"
#include "lib/sdl2w/Logger.h"
#include "lib/sdl2w/Window.h"

#include "../../setupTestUi.h"
#include "ui/SdlPixels.h" // IWYU pragma: keep
#include "ui/UiElement.h"
#include "ui/colors.h"
#include "ui/elements/Quad.h"
#include <memory>

ui::Quad* createBasicQuad(sdl2w::Window* window, int w, int h) {
  auto q = new ui::Quad(window);
  ui::BaseStyle quadStyle;
  quadStyle.x = 0;
  quadStyle.y = 0;
  quadStyle.width = w;
  quadStyle.height = h;
  q->setStyle(quadStyle);
  return q;
}

int main(int argc, char** argv) {
  LOG(INFO) << "Start test" << LOG_ENDL;
  srand(time(NULL));

  std::vector<std::unique_ptr<ui::UiElement>> elements;

  auto _init = [&](sdl2w::Window& window, sdl2w::Store& store) {
    {
      auto q = createBasicQuad(&window, 40, 40);
      q->getStyle().x = 10;
      q->getStyle().y = 10;
      q->setProps({
          .bgColor = ui::Colors::DarkGrey,
          .bgSprite = "actors0_0",
      });
      elements.push_back(std::unique_ptr<ui::UiElement>(q));
    }

    {
      auto q = createBasicQuad(&window, 40, 40);
      q->getStyle().x = 52;
      q->getStyle().y = 10;
      q->getStyle().scale = 2.0f;
      q->setProps({
          .bgColor = ui::Colors::DarkGrey,
          .bgSprite = "actors0_0",
      });
      elements.push_back(std::unique_ptr<ui::UiElement>(q));
    }

    {
      auto q = createBasicQuad(&window, 40, 40);
      q->getStyle().x = 52 + 40 * 2;
      q->getStyle().y = 10;
      q->getStyle().scale = 4.0f;
      q->setProps({
          .bgColor = ui::Colors::DarkGrey,
          .bgSprite = "actors0_0",
      });
      elements.push_back(std::unique_ptr<ui::UiElement>(q));
    }

    // check text and border ----

    {
      auto q = createBasicQuad(&window, 40, 40);
      q->getStyle().x = 10;
      q->getStyle().y = 150;
      q->setProps({
          .bgColor = ui::Colors::DarkBlue,
          .bgSprite = "actors0_2",
          .borderColor = ui::Colors::Red,
          .borderSize = 4,
      });
      elements.push_back(std::unique_ptr<ui::UiElement>(q));
    }

    {
      auto q = createBasicQuad(&window, 40, 40);
      q->getStyle().x = 52;
      q->getStyle().y = 150;
      q->getStyle().scale = 2.0f;
      q->setProps({
          .bgColor = ui::Colors::DarkBlue,
          .bgSprite = "actors0_2",
          .borderColor = ui::Colors::Red,
          .borderSize = 4,
      });
      elements.push_back(std::unique_ptr<ui::UiElement>(q));
    }

    {
      auto q = createBasicQuad(&window, 40, 40);
      q->getStyle().x = 52 + 40 * 2;
      q->getStyle().y = 150;
      q->getStyle().scale = 4.0f;
      q->setProps({
          .bgColor = ui::Colors::DarkBlue,
          .bgSprite = "actors0_2",
          .borderColor = ui::Colors::Red,
          .borderSize = 4,
      });
      elements.push_back(std::unique_ptr<ui::UiElement>(q));
    }

    // Test quad in quad
    {
      auto q = createBasicQuad(&window, 100, 100);
      q->getStyle().x = 330;
      q->getStyle().y = 10;
      q->setProps({
          .bgColor = ui::Colors::DarkBlue,
          // .bgSprite = "actors0_2",
          .borderColor = ui::Colors::Red,
          .borderSize = 1,
      });
      elements.push_back(std::unique_ptr<ui::UiElement>(q));

      auto q2 = createBasicQuad(&window, 75, 75);
      q2->getStyle().x = 10;
      q2->getStyle().y = 10;
      q2->setProps({
          .bgColor = ui::Colors::DarkGrey,
          .bgSprite = "actors0_0",
          .borderColor = ui::Colors::Transparent,
          .borderSize = 0,
      });
      q->addChild(q2);
    }

    {
      auto q = createBasicQuad(&window, 100, 100);
      q->getStyle().x = 330;
      q->getStyle().y = 100;
      q->getStyle().scale = 3.0f;
      q->setProps({
          .bgColor = ui::Colors::DarkBlue,
          // .bgSprite = "actors0_2",
          .borderColor = ui::Colors::Red,
          .borderSize = 1,
      });
      elements.push_back(std::unique_ptr<ui::UiElement>(q));

      auto q2 = createBasicQuad(&window, 75, 75);
      q2->getStyle().x = 10;
      q2->getStyle().y = 10;
      q2->setProps({
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
