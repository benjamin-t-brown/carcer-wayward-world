#pragma once

#include <string>

namespace model {

struct ItemInstance {
  std::string id;
  std::string itemTemplateName;
  int quantity = 1;
};

} // namespace model
