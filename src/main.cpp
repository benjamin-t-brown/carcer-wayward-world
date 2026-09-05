#include <cstdlib>
#include <ctime>
#include <functional>
#include <typeinfo>

import carcer;
import sdl2w;
#include "macros.h"

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
  sdl2w::L10n::init({"en", "la"});
  sdl2w::setupStartupArgs(argc, argv, window);
  sdl2w::L10n::setLanguage(DISABLE_TRANSLATIONS);
  window.getDraw().setBackgroundColor({0, 0, 0});

  sdl2w::AssetLoader assetLoader(window.getDraw(), window.getStore());
  window.getStore().loadAndStoreFont("default", "assets/cabal.ttf");
  window.getStore().loadAndStoreFont("alternate", "assets/monofonto.ttf");

  sdl2w::Draw& d = window.getDraw();
  window.setSoundPct(33);

  auto& events = window.getEvents();
  (void)events;

  auto _initializeLoop = [&]() {
    sdl2w::renderSplash(window);
    return true;
  };

  auto _onInitialized = [&]() {
  };

  auto _mainLoop = [&]() {
    d.setBackgroundColor({10, 10, 10});

#ifndef __EMSCRIPTEN__
    if (window.getEvents().isKeyPressed("Escape")) {
      return false;
    }
#endif

    return true;
  };

  window.startRenderLoop(_initializeLoop, _onInitialized, _mainLoop);
}

int main(int argc, char** argv) {
  LOG(INFO) << "Start program" << LOG_ENDL;
  sdl2w::Window::init();
  srand(static_cast<unsigned>(time(nullptr)));

  runProgram(argc, argv);
  sdl2w::Window::unInit();

  LOG(INFO) << "End program" << LOG_ENDL;
  return 0;
}
