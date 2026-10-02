#include "game/map/DoorLock.h"

#include "bmin/StringInterop.h"

namespace game {
namespace {

bool isLockToolStack(const model::CharacterInventoryItem& stack,
                     const db::Database& database) {
  if (stack.quantity <= 0) {
    return false;
  }
  const auto* itemTemplate = database.findItemTemplate(bmin::toStringView(stack.itemName));
  return itemTemplate != nullptr && itemTemplate->isLockTool;
}

int countLockToolQuantity(const model::CharacterPlayer& leader,
                          const db::Database& database) {
  auto total = int{0};
  for (const auto& stack : leader.inventory) {
    if (!isLockToolStack(stack, database)) {
      continue;
    }
    total += stack.quantity;
  }
  return total;
}

bool inventoryContainsKey(const model::CharacterPlayer& leader,
                          const bmin::String& keyItem,
                          const db::Database& database) {
  if (keyItem.empty()) {
    return false;
  }
  if (database.findItemTemplate(bmin::toStringView(keyItem)) == nullptr) {
    return false;
  }
  for (const auto& stack : leader.inventory) {
    if (stack.itemName == keyItem && stack.quantity > 0) {
      return true;
    }
  }
  return false;
}

void consumeLockTools(model::CharacterPlayer& leader,
                      int quantity,
                      const db::Database& database) {
  auto remaining = quantity;
  auto index = size_t{0};
  while (index < leader.inventory.size() && remaining > 0) {
    auto& stack = leader.inventory[index];
    if (!isLockToolStack(stack, database)) {
      index += 1;
      continue;
    }
    if (stack.quantity > remaining) {
      stack.quantity -= remaining;
      remaining = 0;
    } else {
      remaining -= stack.quantity;
      leader.inventory.erase(index);
    }
  }
}

} // namespace

ClosedDoorOpenResult tryOpenClosedDoor(model::TileInstance& door,
                                       model::CharacterPlayer& leader,
                                       const db::Database& database) {
  if (!door.doorLock.has_value()) {
    door.tileId = door.tileId + 1;
    return ClosedDoorOpenResult::OpenedSilent;
  }

  const auto& lock = *door.doorLock;
  if (lock.lockLevel == 0) {
    if (!inventoryContainsKey(leader, lock.keyItem, database)) {
      return ClosedDoorOpenResult::Blocked;
    }
    door.tileId = door.tileId + 1;
    return ClosedDoorOpenResult::OpenedLockpick;
  }

  const auto toolsRequired = lock.lockLevel - leader.stats.skills.trickery;
  if (toolsRequired <= 0) {
    door.tileId = door.tileId + 1;
    return ClosedDoorOpenResult::OpenedLockpick;
  }

  const auto bashMax = (leader.stats.skills.brutishness / 5) * 5;
  if (lock.lockLevel <= bashMax) {
    door.tileId = door.tileId + 1;
    return ClosedDoorOpenResult::OpenedBash;
  }

  if (countLockToolQuantity(leader, database) < toolsRequired) {
    return ClosedDoorOpenResult::Blocked;
  }

  consumeLockTools(leader, toolsRequired, database);
  door.tileId = door.tileId + 1;
  return ClosedDoorOpenResult::OpenedLockpick;
}

} // namespace game
