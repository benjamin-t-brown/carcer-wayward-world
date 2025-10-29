#pragma once

namespace types {
struct TimerStruct {
  int duration;
  int t;

  TimerStruct(int _duration = 1000) : duration(_duration) {}
  void start(int _duration = 0) {
    t = 0;
    if (_duration > 0) {
      duration = _duration;
    }
  }
  void restart() { t = 0; }
  void update(int deltaTimeMs) { t += deltaTimeMs; }
  bool isComplete() const { return t >= duration; }
  double getPct() const { return static_cast<double>(t) / duration; }
};
} // namespace types