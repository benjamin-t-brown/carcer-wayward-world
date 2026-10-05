#pragma once

#include "bmin/DynArray.h"
#include "bmin/Map.h"
#include "bmin/String.h"

namespace game {

inline constexpr const char kVarsItemsPrefix[] = "vars.items.";
inline constexpr const char kVarsItemsOrderStorageKey[] = "vars.items._order";

bool specialItemQuantityIsPresent(const bmin::String& value);

int parseSpecialItemQuantity(const bmin::String& value);

bmin::DynArray<bmin::String> specialEventItemNamesInDisplayOrder(
    const bmin::Map<bmin::String, bmin::String>& storage);

bool reorderSpecialEventItemInStorage(bmin::Map<bmin::String, bmin::String>& storage,
                                      size_t index,
                                      int direction);

void persistSpecialEventItemOrder(bmin::Map<bmin::String, bmin::String>& storage,
                                  const bmin::DynArray<bmin::String>& orderedNames);

} // namespace game
