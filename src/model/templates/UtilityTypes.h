#pragma once

#include "bmin/String.h"

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
