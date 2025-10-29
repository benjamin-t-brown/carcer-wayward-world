#pragma once

#include <cstdlib>
#include <string>

namespace model {
struct TimerStruct {
  int duration;
  int t = 0;

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

inline std::string createRandomId() {
  return std::to_string((rand() % 1000000) + (rand() % 1000000) + (rand() % 1000000));
}

} // namespace model