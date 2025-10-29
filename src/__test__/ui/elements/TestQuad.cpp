#include "lib/sdl2w/Defines.h"
#include "lib/sdl2w/Draw.h"
#include "lib/sdl2w/Logger.h"
#include "lib/sdl2w/Window.h"

#include "../../setupTestUi.h"
#include "ui/UiElement.h"
#include "ui/elements/Quad.h"
#include "ui/elements/TextLine.h"
#include "ui/elements/TextParagraph.h"
#include <SDL2/SDL_pixels.h>
#include <memory>

int main(int argc, char** argv) {
  LOG(INFO) << "Start test" << LOG_ENDL;
  sdl2w::Window::init();
  srand(time(NULL));

  std::vector<std::unique_ptr<ui::UiElement>> elements;

  auto _init = [&](sdl2w::Window& window, sdl2w::Store& store) {
    LOG(INFO) << "STUFF INITTED" << LOG_ENDL;
    auto q = std::make_unique<ui::Quad>(&window);
    ui::BaseStyle quadStyle;
    quadStyle.x = 10;
    quadStyle.y = 10;
    quadStyle.width = 200;
    quadStyle.height = 200;
    q->setStyle(quadStyle);
    ui::QuadProps quadProps;
    quadProps.bgColor = SDL_Color{100, 100, 100, 255};
    quadProps.borderColor = SDL_Color{255, 0, 0, 255};
    quadProps.borderSize = 4;
    q->setProps(quadProps);

    {
      auto text = std::make_unique<ui::TextLine>(&window, q.get());
      ui::BaseStyle textStyle;
      textStyle.x = 10;
      textStyle.y = 0;
      textStyle.fontSize = sdl2w::TEXT_SIZE_24;
      textStyle.fontFamily = ui::FontFamily::PARAGRAPH;
      text->setStyle(textStyle);
      ui::TextLineProps textProps;
      textProps.textBlocks.push_back({"Hello "});
      textProps.textBlocks.push_back({.text = "World",
                                      .fontSize = sdl2w::TEXT_SIZE_20,
                                      .fontColor = SDL_Color{0, 0, 255}});
      text->setProps(textProps);
      q->getChildren().push_back(std::move(text));
    }

    {
      auto paragraph = std::make_unique<ui::TextParagraph>(&window);

      ui::BaseStyle style;
      style.x = 0;
      style.y = 40;
      style.width = quadStyle.width;
      style.height = quadStyle.height;
      style.fontSize = sdl2w::TEXT_SIZE_16;
      style.fontColor = SDL_Color{255, 255, 255, 255};
      paragraph->setStyle(style);
      ui::TextParagraphProps props;
      props.textBlocks.push_back({"Hello. "});
      // props.textBlocks.push_back({"\nNew line with different style!",
      //                             ui::FontFamily::H1,
      //                             sdl2w::TEXT_SIZE_20,
      //                             SDL_Color{255, 0, 0, 255}});
      q->getChildren().push_back(std::move(paragraph));
    }

    elements.push_back(std::move(q));
  };

  auto _updateRender = [&](sdl2w::Window& window, sdl2w::Store& store) {
    for (auto& elem : elements) {
      if (elem) {
        elem->render(window.getDeltaTime());
      }
    }
    return true;
  };

  setupTestUi(argc, argv, TestUiParams{640, 480}, _init, _updateRender);
  LOG(INFO) << "End test" << LOG_ENDL;
  sdl2w::Window::unInit();
  return 0;
}
