#include "model/UtilityTypes.h"

#include <cstdlib>

namespace model {

TimerStruct::TimerStruct(int duration) : duration(duration) {}

std::string createRandomId() {
  return std::to_string((rand() % 1000000) + (rand() % 1000000) + (rand() % 1000000));
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
