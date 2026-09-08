#pragma once

#include "bmin/String.h"
#include "sdl2w/AssetLoader.h"
#include "sdl2w/Draw.h"
#include "sdl2w/Events.h"
#include "sdl2w/Init.h"
#include "sdl2w/L10n.h"
#include "sdl2w/Logger.h"
#include "sdl2w/Window.h"
#include <functional>

struct TestUiParams {
  int width;
  int height;
  bmin::String title = "UI Test";
};

// Runs after the render loop exits, while window/store are still alive.
// Clear UI elements here so Quad and other SDL-owned resources are released
// before the window is destroyed and SDL_Quit is called.
inline void setupTestUi(int argc,
                        char** argv,
                        const TestUiParams& params,
                        std::function<void(sdl2w::Window&, sdl2w::Store&)> _init,
                        std::function<bool(sdl2w::Window&, sdl2w::Store&)> _updateRender,
                        std::function<void()> _teardown = {}) {
  LOG(INFO) << "Starting UI Test: " << params.title << LOG_ENDL;
  sdl2w::Window::init();

  {
    sdl2w::Store store;
    sdl2w::Window window(store,
                         {
                             .mode = sdl2w::DrawMode::GPU,
                             .title = params.title.cStr(),
                             .w = params.width,
                             .h = params.height,
                             .x = 25,
                             .y = 50,
                             .renderW = params.width,
                             .renderH = params.height,
                         });

    sdl2w::L10n::init({{"en"}});
    sdl2w::setupStartupArgs(argc, argv, window);
    sdl2w::L10n::setLanguage(DISABLE_TRANSLATIONS);
    window.getDraw().setBackgroundColor({0, 0, 0});

    sdl2w::AssetLoader assetLoader(window.getDraw(), window.getStore());
    window.getStore().loadAndStoreFont("title", "assets/squealer.ttf");
    window.getStore().loadAndStoreFont("default", "assets/monofonto.ttf");
    window.getStore().loadAndStoreFont("alternate", "assets/monofonto.ttf");
    window.getStore().loadAndStoreFont("text", "assets/notosans-regular.ttf");
    window.getStore().loadAndStoreFont("text-bold", "assets/notosans-bold.ttf");
    assetLoader.loadAssetsFromFile(sdl2w::ASSET_FILE, "assets/assets.ui.txt");
    assetLoader.loadAssetsFromFile(sdl2w::ASSET_FILE, "assets/assets.game.txt");

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
      window.getDraw().setBackgroundColor({60, 60, 60});

#ifndef __EMSCRIPTEN__
      if (window.getEvents().isKeyPressed("q") ||
          window.getEvents().isKeyPressed("Q")) {
        return false;
      }
#endif

      return _updateRender(window, store);
    };

    window.startRenderLoop(_initializeLoop, _onInitialized, _mainLoop);

    if (_teardown) {
      _teardown();
    }
  }

  sdl2w::Window::unInit();
}
