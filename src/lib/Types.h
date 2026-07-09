#pragma once

#include "bmin/DynArray.h"       // IWYU pragma: keep
#include "bmin/List.h"           // IWYU pragma: keep
#include "bmin/String.h"         // IWYU pragma: keep
#include "bmin/StringInterop.h"  // IWYU pragma: keep
#include "bmin/StringStream.h"   // IWYU pragma: keep
#include "bmin/UniquePtr.h"      // IWYU pragma: keep

#include <string_view>

using String = bmin::String;
using StringStream = bmin::StringStream;

template <typename T>
using DynArray = bmin::DynArray<T>;

template <typename T>
using List = bmin::List<T>;

template <typename T, typename Deleter = bmin::DefaultDelete<T>>
using UniquePtr = bmin::UniquePtr<T, Deleter>;

using bmin::makeUnique;

inline String stringFromView(std::string_view sv) {
  return String(sv.data(), sv.size());
}
