#pragma once

#include "lib/sdl2w/AssetLoader.h"
#include "lib/sdl2w/Draw.h"
#include "lib/sdl2w/Events.h"
#include "lib/sdl2w/Init.h"
#include "lib/sdl2w/L10n.h"
#include "lib/sdl2w/Logger.h"
#include <functional>

struct TestUiParams {
  int width;
  int height;
  std::string title = "UI Test";
};

inline void
setupTestUi(int argc,
            char** argv,
            const TestUiParams& params,
            std::function<void(sdl2w::Window&, sdl2w::Store&)> _init,
            std::function<bool(sdl2w::Window&, sdl2w::Store&)> _updateRender) {
  LOG(INFO) << "Starting UI Test: " << params.title << LOG_ENDL;
  sdl2w::Window::init();

  sdl2w::Store store;
  sdl2w::Window window(store,
                       {
                           .mode = sdl2w::DrawMode::GPU,
                           .title = params.title,
                           .w = params.width,
                           .h = params.height,
                           .x = 25,
                           .y = 50,
                           .renderW = params.width,
                           .renderH = params.height,
                       });

  sdl2w::L10n::init(std::vector<std::string>({"en"}));
  sdl2w::setupStartupArgs(argc, argv, window);
  sdl2w::L10n::setLanguage("default");
  window.getDraw().setBackgroundColor({0, 0, 0});

  sdl2w::AssetLoader assetLoader(window.getDraw(), window.getStore());
  window.getStore().loadAndStoreFont("default", "assets/monofonto.ttf");
  window.getStore().loadAndStoreFont("monofonto", "assets/cabal.ttf");

  window.setSoundPct(33);

  auto _initializeLoop = [&]() {
    sdl2w::renderSplash(window);
    return true;
  };

  auto _onInitialized = [&]() {
    LOG(INFO) << "Initializing test..." << LOG_ENDL;
    _init(window, store);
  };

  auto _mainLoop = [&]() {
    window.getDraw().setBackgroundColor({10, 10, 10});

#ifndef __EMSCRIPTEN__
    if (window.getEvents().isKeyPressed("Escape")) {
      return false;
    }
#endif

    return _updateRender(window, store);
  };

  window.startRenderLoop(_initializeLoop, _onInitialized, _mainLoop);
}
