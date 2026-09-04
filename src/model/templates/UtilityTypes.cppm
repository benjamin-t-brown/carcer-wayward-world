module;
#include <cstddef>
#include <cstdint>
#include <utility>
#include <cstdlib>

export module carcer.model.templates.UtilityTypes;
export import bmin.containers;
import bmin.string_interop;
import sdl2w;
#include "macros.h"

export {

// --- from model/templates/UtilityTypes.h ---
namespace model {

struct TimerStruct {
  int duration = 1000;
  int t = 0;

  TimerStruct(int duration = 1000);
};

bmin::String createRandomId();
void timerStructStart(TimerStruct& timer, int duration = 0);
void timerStructRestart(TimerStruct& timer);
void timerStructUpdate(TimerStruct& timer, int deltaTimeMs);
bool timerStructIsComplete(const TimerStruct& timer);
double timerStructGetPct(const TimerStruct& timer);

} // namespace model

} // export

namespace model {

TimerStruct::TimerStruct(int duration) : duration(duration) {}

bmin::String createRandomId() {
  return bmin::toString((rand() % 1000000) + (rand() % 1000000) + (rand() % 1000000));
}

void timerStructStart(TimerStruct& timer, int duration) {
  timer.t = 0;
  if (duration > 0) {
    timer.duration = duration;
  }
}

void timerStructRestart(TimerStruct& timer) { timer.t = 0; }

void timerStructUpdate(TimerStruct& timer, int deltaTimeMs) { timer.t += deltaTimeMs; }

bool timerStructIsComplete(const TimerStruct& timer) { return timer.t >= timer.duration; }

double timerStructGetPct(const TimerStruct& timer) {
  return static_cast<double>(timer.t) / timer.duration;
}

} // namespace model
