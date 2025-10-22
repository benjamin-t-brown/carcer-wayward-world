#include "../setupTestUi.h"
#include "lib/sdl2w/Defines.h"
#include "lib/sdl2w/Draw.h"
#include "lib/sdl2w/Logger.h"
#include "lib/sdl2w/Window.h"
#include "ui/UiElement.h"
#include "ui/elements/Quad.h"
#include "ui/elements/TextParagraph.h"
#include <SDL2/SDL_pixels.h>
#include <memory>

struct QuadWithParagraphParams {
  int x = 0;
  int y = 0;
  int width = 300;
  int height = 200;
  SDL_Color bgColor = SDL_Color{50, 50, 50, 255};
  SDL_Color borderColor = SDL_Color{0, 0, 0, 0};
  int borderSize = 0;
  std::vector<ui::TextBlock> textBlocks;
  sdl2w::TextSize fontSize = sdl2w::TEXT_SIZE_16;
  SDL_Color fontColor = SDL_Color{255, 255, 255, 255};
  int lineSpacing = 0;
  int padding = 0;
};

std::unique_ptr<ui::Quad>
createQuadWithParagraph(sdl2w::Window* window,
                        const QuadWithParagraphParams& params) {
  auto quad = std::make_unique<ui::Quad>(window);

  // Set quad style
  ui::BaseStyle quadStyle;
  quadStyle.x = params.x;
  quadStyle.y = params.y;
  quadStyle.width = params.width;
  quadStyle.height = params.height;
  quad->setStyle(quadStyle);

  // Set quad properties
  ui::QuadProps quadProps;
  quadProps.bgColor = params.bgColor;
  quadProps.borderColor = params.borderColor;
  quadProps.borderSize = params.borderSize;
  quad->setProps(quadProps);

  // Create paragraph inside the quad
  auto paragraph = std::make_unique<ui::TextParagraph>(window);

  // Set paragraph style
  ui::BaseStyle paragraphStyle;
  paragraphStyle.x = params.padding;
  paragraphStyle.y = params.padding;
  paragraphStyle.width = params.width - (params.padding * 2);
  paragraphStyle.height = params.height - (params.padding * 2);
  paragraphStyle.fontSize = params.fontSize;
  paragraphStyle.fontColor = params.fontColor;
  paragraphStyle.lineSpacing = params.lineSpacing;
  paragraph->setStyle(paragraphStyle);

  // Set paragraph text blocks
  ui::TextParagraphProps paragraphProps;
  paragraphProps.textBlocks = params.textBlocks;
  paragraph->setProps(paragraphProps);

  // Add paragraph as child of quad
  quad->getChildren().push_back(std::move(paragraph));

  return quad;
}

int main(int argc, char** argv) {
  LOG(INFO) << "Start test" << LOG_ENDL;
  sdl2w::Window::init();
  srand(time(NULL));

  std::vector<std::unique_ptr<ui::UiElement>> elements;

  auto _init = [&](sdl2w::Window& window, sdl2w::Store& store) {
    LOG(INFO) << "STUFF INITTED" << LOG_ENDL;

    // Example 1: Simple paragraph
    QuadWithParagraphParams params1;
    params1.x = 50;
    params1.y = 50;
    params1.width = window.getDims().first - 100;
    params1.height = 150;
    params1.bgColor = SDL_Color{50, 100, 50, 255};
    params1.borderColor = SDL_Color{100, 150, 100, 255};
    params1.borderSize = 2;
    params1.fontSize = sdl2w::TEXT_SIZE_20;
    params1.fontColor = SDL_Color{255, 255, 255, 255};
    params1.lineSpacing = 4;
    params1.padding = 10;
    params1.textBlocks.push_back(
        {"Hello World And Friends. This is a dire situation about the great brown fox "
         "jumping over the lazy dog!"});

    auto quad1 = createQuadWithParagraph(&window, params1);
    elements.push_back(std::move(quad1));

    // Example 2: Multi-styled paragraph
    QuadWithParagraphParams params2;
    params2.x = 50;
    params2.y = 220;
    params2.width = window.getDims().first - 100;
    params2.height = 200;
    params2.bgColor = SDL_Color{50, 50, 100, 255};
    params2.borderColor = SDL_Color{100, 100, 150, 255};
    params2.borderSize = 3;
    params2.fontSize = sdl2w::TEXT_SIZE_16;
    params2.fontColor = SDL_Color{220, 220, 220, 255};
    params2.lineSpacing = 10;
    params2.padding = 15;
    
    ui::TextBlock block1;
    block1.text = "This is a paragraph with ";
    params2.textBlocks.push_back(block1);
    
    ui::TextBlock block2;
    block2.text = "multiple styles! ";
    block2.fontSize = sdl2w::TEXT_SIZE_24;
    block2.fontColor = SDL_Color{255, 200, 0, 255};
    params2.textBlocks.push_back(block2);
    
    ui::TextBlock block3;
    block3.text = "You can mix different font sizes and colors in the same paragraph.";
    params2.textBlocks.push_back(block3);

    ui::TextBlock block4;
    block4.text = "And it also supports\nnewlines!\nisn't that cool!";
    params2.textBlocks.push_back(block4);

    auto quad2 = createQuadWithParagraph(&window, params2);
    elements.push_back(std::move(quad2));
  };

  auto _updateRender = [&](sdl2w::Window& window, sdl2w::Store& store) {
    for (auto& elem : elements) {
      if (elem) {
        elem->render();
      }
    }
    return true;
  };

  setupTestUi(argc, argv, TestUiParams{640, 480}, _init, _updateRender);
  LOG(INFO) << "End test" << LOG_ENDL;
  sdl2w::Window::unInit();

  return 0;
}
