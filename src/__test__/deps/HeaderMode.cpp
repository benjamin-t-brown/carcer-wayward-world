#include "Events.h"
#include "bmin/String.h"

int main() {
  bmin::String label{"header-mode"};
  sdl2w::EventRoute route;
  (void)route;
  return label.size() == 11 ? 0 : 1;
}
