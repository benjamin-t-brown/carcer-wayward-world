#pragma once

#include "bmin/String.h"

namespace model {

struct ItemInstance {
  bmin::String id;
  bmin::String itemTemplateName;
  int quantity = 1;
  int x = 0;
  int y = 0;
};

} // namespace model
