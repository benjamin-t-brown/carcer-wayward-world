#include "LibHTML.h"
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#include <string>
#endif

namespace libhtml {

void notifyGameStarted() {
#ifdef __EMSCRIPTEN__
  const std::string script = std::string("window.Lib.notifyGameStarted()");
  emscripten_run_script(script.c_str());
#endif
}
void notifyGameReady() {
#ifdef __EMSCRIPTEN__
  const std::string script = std::string("window.Lib.notifyGameReady()");
  emscripten_run_script(script.c_str());
#endif
}
void notifyGameCompleted(bool isVictory) {
#ifdef __EMSCRIPTEN__
  const std::string script = std::string("window.Lib.notifyGameCompleted(" +
                                         std::to_string(isVictory) + ")");
  emscripten_run_script(script.c_str());
#endif
}
} // namespace libhtml