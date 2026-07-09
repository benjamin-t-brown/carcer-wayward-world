#pragma once

#include "lib/Types.h"

namespace model {

struct ItemInstance {
  String id;
  String itemTemplateName;
  int quantity = 1;
};

} // namespace model
