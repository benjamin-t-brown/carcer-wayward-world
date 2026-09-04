module;
#include <cstddef>
#include <cstdint>
#include <utility>

export module carcer.model.instances.ItemInstance;
export import bmin.containers;
import bmin.string_interop;

export {

// --- from model/instances/ItemInstance.h ---
namespace model {

struct ItemInstance {
  bmin::String id;
  bmin::String itemTemplateName;
  int quantity = 1;
  int x = 0;
  int y = 0;
};

} // namespace model

} // export
