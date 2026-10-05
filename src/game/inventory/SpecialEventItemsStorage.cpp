#include "game/inventory/SpecialEventItemsStorage.h"
#include "in3/EventRunnerHelpers.h"
#include "bmin/StringInterop.h"
#include <cstdlib>

namespace game {

namespace {

bool isSpecialItemStorageKey(std::string_view keyView) {
  if (!keyView.starts_with(kVarsItemsPrefix)) {
    return false;
  }
  const auto itemNameView = keyView.substr(std::char_traits<char>::length(kVarsItemsPrefix));
  return !itemNameView.empty() && itemNameView != "_order";
}

bmin::String joinItemNames(const bmin::DynArray<bmin::String>& names) {
  bmin::String joined;
  for (size_t i = 0; i < names.size(); ++i) {
    if (i > 0) {
      joined += ",";
    }
    joined += names[i];
  }
  return joined;
}

} // namespace

bool specialItemQuantityIsPresent(const bmin::String& value) {
  return !value.empty() && value != "0" && value != "false";
}

int parseSpecialItemQuantity(const bmin::String& value) {
  char* end = nullptr;
  const long parsed = std::strtol(value.cStr(), &end, 10);
  if (end == value.cStr() || parsed <= 0) {
    return 1;
  }
  return static_cast<int>(parsed);
}

bmin::DynArray<bmin::String> specialEventItemNamesInDisplayOrder(
    const bmin::Map<bmin::String, bmin::String>& storage) {
  bmin::DynArray<bmin::String> presentNames;
  for (const auto& entry : storage) {
    const auto keyView = bmin::toStringView(entry.key);
    if (!isSpecialItemStorageKey(keyView)) {
      continue;
    }
    if (!specialItemQuantityIsPresent(entry.value)) {
      continue;
    }
    const auto itemNameView =
        keyView.substr(std::char_traits<char>::length(kVarsItemsPrefix));
    presentNames.pushBack(bmin::String(itemNameView.data(), itemNameView.size()));
  }

  bmin::DynArray<bmin::String> ordered;
  if (const auto orderValue = in3::getStorage(storage, bmin::String(kVarsItemsOrderStorageKey))) {
    const auto segments = in3::splitString(*orderValue, ',');
    for (const auto& segment : segments) {
      if (segment.empty()) {
        continue;
      }
      for (const auto& name : presentNames) {
        if (name == segment) {
          ordered.pushBack(name);
          break;
        }
      }
    }
  }

  for (const auto& name : presentNames) {
    bool alreadyListed = false;
    for (const auto& listed : ordered) {
      if (listed == name) {
        alreadyListed = true;
        break;
      }
    }
    if (!alreadyListed) {
      ordered.pushBack(name);
    }
  }

  return ordered;
}

void persistSpecialEventItemOrder(bmin::Map<bmin::String, bmin::String>& storage,
                                  const bmin::DynArray<bmin::String>& orderedNames) {
  if (orderedNames.empty()) {
    return;
  }
  in3::setStorage(storage, bmin::String(kVarsItemsOrderStorageKey),
                  joinItemNames(orderedNames));
}

bool reorderSpecialEventItemInStorage(bmin::Map<bmin::String, bmin::String>& storage,
                                      size_t index,
                                      int direction) {
  auto ordered = specialEventItemNamesInDisplayOrder(storage);
  if (index >= ordered.size()) {
    return false;
  }
  if (direction < 0) {
    if (index == 0) {
      return false;
    }
    std::swap(ordered[index], ordered[index - 1]);
  } else if (direction > 0) {
    if (index + 1 >= ordered.size()) {
      return false;
    }
    std::swap(ordered[index], ordered[index + 1]);
  } else {
    return false;
  }
  persistSpecialEventItemOrder(storage, ordered);
  return true;
}

} // namespace game
