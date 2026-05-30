#pragma once

#include <string>

namespace model {

struct TimerStruct {
  int duration = 1000;
  int t = 0;

  TimerStruct(int duration = 1000);
};

std::string createRandomId();
void timerStructStart(TimerStruct& timer, int duration = 0);
void timerStructRestart(TimerStruct& timer);
void timerStructUpdate(TimerStruct& timer, int deltaTimeMs);
bool timerStructIsComplete(const TimerStruct& timer);
double timerStructGetPct(const TimerStruct& timer);

} // namespace model
