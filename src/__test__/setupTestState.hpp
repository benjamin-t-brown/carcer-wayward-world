#pragma once

#include "bmin/String.h"
#include "bmin/StringInterop.h"
#include "db/Database.h"
#include "lib/Json.h"
#include "sdl2w/AssetLoader.h"
#include "sdl2w/Logger.h"
#include "state/State.hpp"
#include <stdexcept>

namespace {

// ceditor EventRunner stores nested objects; C++ specialEventStorage is flat
// dotted keys (vars.quests.X.step, once.id, tmp.id).
inline bmin::String jsonLeafToStorageString(const Json& value) {
  if (value.is_string()) {
    return value.get<bmin::String>();
  }
  if (value.is_boolean()) {
    return value.get<bool>() ? "true" : "false";
  }
  if (value.is_number_integer()) {
    return bmin::toString(value.get<int>());
  }
  if (value.is_number()) {
    return bmin::toString(value.get<double>());
  }
  throw std::runtime_error("in3 storage leaf must be string, number, or boolean");
}

inline void flattenIn3StorageObject(const Json& obj,
                                    const bmin::String& prefix,
                                    bmin::Map<bmin::String, bmin::String>& out) {
  if (!obj.is_object()) {
    throw std::runtime_error(
        (bmin::String("in3 storage node must be an object: ") + prefix).cStr());
  }
  for (const auto& [key, value] : obj.items()) {
    const bmin::String fullKey = prefix.empty() ? key : prefix + "." + key;
    if (value.is_object()) {
      flattenIn3StorageObject(value, fullKey, out);
      continue;
    }
    out[fullKey] = jsonLeafToStorageString(value);
  }
}

// Accepts ceditor Edit State JSON:
// { "vars": {...}, "once": {...}, "tmp": {...} }
inline void applyIn3StorageJson(const Json& storageRoot,
                                bmin::Map<bmin::String, bmin::String>& storage) {
  static constexpr const char* kRoots[] = {"vars", "once", "tmp"};
  auto applied = false;
  for (const char* rootKey : kRoots) {
    if (!storageRoot.contains(rootKey) || !storageRoot[rootKey].is_object()) {
      continue;
    }
    flattenIn3StorageObject(storageRoot[rootKey], rootKey, storage);
    applied = true;
  }
  if (!applied) {
    throw std::runtime_error(
        "in3 state JSON needs at least one of vars / once / tmp objects");
  }
}

} // namespace

// Load ceditor-style in3 storage into state.specialEventStorage.
// Nested vars/once/tmp become flat dotted keys.
inline void loadTestStateFromJson(const bmin::String& path,
                                  db::Database& /*database*/,
                                  state::State& state) {
  LOG(INFO) << "Loading test in3 state fixture: " << path << LOG_ENDL;
  const bmin::String fileContent = sdl2w::loadFileAsString(bmin::toStringView(path));
  Json root;
  try {
    root = Json::parse(fileContent.cStr(), nullptr, true, true);
  } catch (const Json::parse_error& e) {
    throw std::runtime_error(
        (bmin::String("Failed to parse test state JSON: ") + e.what()).cStr());
  }
  if (!root.is_object()) {
    throw std::runtime_error("Test state JSON must be an object");
  }

  applyIn3StorageJson(root, state.specialEventStorage);
}
