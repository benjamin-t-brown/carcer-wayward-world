#include "../../setupTestUi.h"
#include "sdl2w/Defines.h"
#include "sdl2w/Draw.h"
#include "sdl2w/Logger.h"
#include "sdl2w/Window.h"
#include "ui/SdlPixels.h" // IWYU pragma: keep
#include "ui/TextStyle.h"
#include "ui/UiElement.h"
#include "ui/elements/Quad.h"
#include "ui/elements/TextParagraph.h"
#include <memory>
#include "bmin/DynArray.h"
#include "bmin/UniquePtr.h"

// struct QuadWithParagraphParams {
//   int x = 0;
//   int y = 0;
//   int width = 300;
//   int height = 200;
//   SDL_Color bgColor = SDL_Color{50, 50, 50, 255};
//   SDL_Color borderColor = SDL_Color{0, 0, 0, 0};
//   int borderSize = 0;
//   bmin::DynArray<ui::TextBlock> textBlocks;
//   sdl2w::TextSize fontSize = sdl2w::TEXT_SIZE_16;
//   SDL_Color fontColor = SDL_Color{255, 255, 255, 255};
//   int lineSpacing = 0;
//   int padding = 0;
// };

// bmin::UniquePtr<ui::Quad>
// createQuadWithParagraph(sdl2w::Window* window,
//                         const QuadWithParagraphParams& params) {
//   auto quad = bmin::makeUnique<ui::Quad>(window);
//
//   quad->setPos(params.x, params.y);
//   quad->setProps(ui::QuadProps{
//       .width = params.width,
//       .height = params.height,
//       .bgColor = params.bgColor,
//       .borderColor = params.borderColor,
//       .borderSize = params.borderSize,
//   });
//
//   auto paragraph = bmin::makeUnique<ui::TextParagraph>(window);
//   paragraph->setPos(params.padding, params.padding);
//   paragraph->setProps(ui::TextParagraphProps{
//       .textBlocks = params.textBlocks,
//       .width = params.width - (params.padding * 2),
//       .lineSpacing = params.lineSpacing,
//       .fontSize = params.fontSize,
//       .fontColor = params.fontColor,
//   });
//
//   quad->getChildren().pushBack(std::move(paragraph));
//
//   return quad;
// }

int main(int argc, char** argv) {
  LOG(INFO) << "Start test" << LOG_ENDL;
  srand(time(NULL));

  bmin::DynArray<bmin::UniquePtr<ui::UiElement>> elements;

  auto _init = [&](sdl2w::Window& window, sdl2w::Store& store) {
    LOG(INFO) << "STUFF INITTED" << LOG_ENDL;

    auto textParagraph = new ui::TextParagraph(&window, nullptr);
    ui::TextFontProps font;
    ui::setBaseFontConfig(font, ui::BaseFontConfig::MODAL_TEXT);
    textParagraph->setPos(25, 25);
    textParagraph->setScale(1.f);
    textParagraph->setProps(ui::TextParagraphProps{
        .textBlocks =
            {
                {.text = "According to all known laws of aviation, there is no way a bee "
                         "should be able to fly.\n"},
                {.text = "Its wings are too small to get its fat little body off the "
                         "ground.\n"},
                {.text = "The bee, of course, flies anyway because bees don't care what "
                         "humans think is impossible.\n"},
                {.text = "Yellow, black. Yellow, black. Yellow, black. Yellow, black.\n"},
                {.text = "Ooh, black and yellow!\n"},
                {.text = "Let's shake it up a little.\n"},
                {.text = "Barry! Breakfast is ready!\n"},
            },
        .width = window.getDims().first - 100,
        .bgColor = ui::Colors::OffWhite,
        .padding = 0,
        .lineSpacing = 0,
        .fontFamily = font.fontFamily,
        .fontSize = sdl2w::TEXT_SIZE_20,
        .fontColor = ui::Colors::Black,
    });
    elements.pushBack(bmin::UniquePtr<ui::UiElement>(textParagraph));
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
