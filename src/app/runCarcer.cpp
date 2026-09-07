#include "app/runCarcer.h"

#include "db/Database.h"
#include "layers/LayerManager.h"
#include "sdl2w/AssetLoader.h"
#include "sdl2w/Draw.h"
#include "sdl2w/Init.h"
#include "sdl2w/L10n.h"
#include "sdl2w/Logger.h"
#include "sdl2w/Store.h"
#include "sdl2w/Window.h"
#include "state/DatabaseInterface.h"
#include "state/LayerRequest.h"
#include "state/StateManager.h"
#include <cstdlib>
#include <ctime>

namespace {

void runProgram(int argc, char** argv) {
  constexpr int width = 640;
  constexpr int height = 480;

  sdl2w::Store store;
  sdl2w::Window window(store,
                       {
                           .mode = sdl2w::DrawMode::GPU,
                           .title = "Carcer",
                           .w = width,
                           .h = height,
                           .x = 25,
                           .y = 50,
                           .renderW = width,
                           .renderH = height,
                       });
  sdl2w::L10n::init({"en", "la"});
  sdl2w::setupStartupArgs(argc, argv, window);
  sdl2w::L10n::setLanguage(DISABLE_TRANSLATIONS);
  window.getDraw().setBackgroundColor({0, 0, 0});

  sdl2w::AssetLoader assetLoader(window.getDraw(), window.getStore());
  window.getStore().loadAndStoreFont("default", "assets/cabal.ttf");
  window.getStore().loadAndStoreFont("alternate", "assets/monofonto.ttf");
  window.setSoundPct(33);

  db::Database database;
  database.load();
  state::DatabaseInterface::setDatabase(&database);
  state::StateManager stateManager;
  state::pushLayerRequest(stateManager.getState(),
                          state::LayerRequest{.id = state::LayerId::World});
  layers::LayerManager layerManager(&window);
  layerManager.start();
  state::DatabaseInterface::setDatabase(nullptr);
}

} // namespace

int runCarcer(int argc, char** argv) {
  LOG(INFO) << "Start program" << LOG_ENDL;
  sdl2w::Window::init();
  std::srand(static_cast<unsigned>(std::time(nullptr)));

  runProgram(argc, argv);

  sdl2w::Window::unInit();
  LOG(INFO) << "End program" << LOG_ENDL;
  return 0;
}
