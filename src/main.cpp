#include "lib/hiscore/hiscore.h"
#include "lib/sdl2w/AssetLoader.h"
#include "lib/sdl2w/Draw.h"
#include "lib/sdl2w/Events.h"
#include "lib/sdl2w/Init.h"
#include "lib/sdl2w/L10n.h"
#include "lib/sdl2w/Logger.h"

void runProgram(int argc, char** argv) {
  const int w = 640;
  const int h = 480;

  sdl2w::Store store;
  sdl2w::Window window(store,
                       {
                           .mode = sdl2w::DrawMode::GPU,
                           .title = "Carcer",
                           .w = w,
                           .h = h,
                           .x = 25, // SDL_WINDOWPOS_UNDEFINED
                           .y = 50, // SDL_WINDOWPOS_UNDEFINED
                           .renderW = w,
                           .renderH = h,
                       });
  sdl2w::L10n::init(std::vector<std::string>({"en", "la"}));
  sdl2w::setupStartupArgs(argc, argv, window);
  sdl2w::L10n::setLanguage("default");
  window.getDraw().setBackgroundColor({0, 0, 0});

  sdl2w::AssetLoader assetLoader(window.getDraw(), window.getStore());
  window.getStore().loadAndStoreFont("default", "assets/monofonto.ttf");
  window.getStore().loadAndStoreFont("monofonto", "assets/cabal.ttf");

  sdl2w::Draw& d = window.getDraw();
  window.setSoundPct(33);

  auto& events = window.getEvents();
  // events.setKeyboardEvent(
  //     sdl2w::ON_KEY_PRESS,
  //     [&](const std::string& key, int) { game.handleKeyPress(key); });

  auto _initializeLoop = [&]() {
    sdl2w::renderSplash(window);
    return true;
  };

  auto _onInitialized = [&]() {
    // load high scores
    // auto hiscores = hiscore::getHighScores();
    // if (hiscores.size()) {
    //   game.state.wins = hiscores[0].score;
    // }

    // game.start();
  };

  auto _mainLoop = [&]() {
    d.setBackgroundColor({10, 10, 10});

#ifndef __EMSCRIPTEN__
    if (window.getEvents().isKeyPressed("Escape")) {
      return false;
    }
#endif

    // game.update(std::min(window.getDeltaTime(), 100));
    // game.render();
    return true;
  };

  window.startRenderLoop(_initializeLoop, _onInitialized, _mainLoop);
}

int main(int argc, char** argv) {
  LOG(INFO) << "Start program" << LOG_ENDL;
  sdl2w::Window::init();
  srand(time(NULL));

  runProgram(argc, argv);

  sdl2w::Window::unInit();
  LOG(INFO) << "End program" << LOG_ENDL;

  return 0;
}
