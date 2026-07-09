#pragma once

#include "Types.h"
#include "bmin/DynArray.h"
#include "bmin/Map.h"

#include <cstddef>
#include <cstdint>
#include <exception>

class Json {
 public:
  class parse_error : public std::exception {
   public:
    explicit parse_error(String message);
    const char* what() const noexcept override;

   private:
    String _message;
  };

  Json();
  Json(const Json& other);
  Json(Json&& other) noexcept;
  Json& operator=(Json other);
  ~Json();

  Json& operator=(String value);
  Json& operator=(const char* value);
  Json& operator=(int value);
  Json& operator=(bool value);

  bool is_object() const;
  bool is_array() const;
  bool is_string() const;
  bool is_number_integer() const;
  bool is_boolean() const;

  const Json& operator[](const char* key) const;
  Json& operator[](const char* key);
  const Json& operator[](int index) const;
  Json& operator[](int index);
  const Json& operator[](size_t index) const;
  Json& operator[](size_t index);

  bool contains(const char* key) const;

  template <typename T>
  T get() const;

  String value(const char* key, String defaultValue) const;
  int value(const char* key, int defaultValue) const;
  bool value(const char* key, bool defaultValue) const;

  size_t size() const;

  const Json* begin() const;
  const Json* end() const;

  struct JsonObjectItem {
    const String& key;
    const Json& value;
  };

  class JsonObjectIterator {
   public:
    JsonObjectIterator();
    JsonObjectIterator(bmin::Map<String, Json>* map, bmin::Map<String, Json>::Iterator it,
                       bmin::Map<String, Json>::Iterator endIt);

    JsonObjectItem operator*() const;
    JsonObjectIterator& operator++();
    bool operator==(const JsonObjectIterator& other) const;
    bool operator!=(const JsonObjectIterator& other) const;

   private:
    bmin::Map<String, Json>* _map = nullptr;
    bmin::Map<String, Json>::Iterator _it;
    bmin::Map<String, Json>::Iterator _end;
    bool _valid = false;
  };

  class JsonObjectItemRange {
   public:
    explicit JsonObjectItemRange(const Json& json);

    JsonObjectIterator begin() const;
    JsonObjectIterator end() const;

   private:
    const Json& _json;
  };

  JsonObjectItemRange items() const;

  static Json parse(const char* text, void* callback, bool allowExceptions,
                    bool ignoreComments);

 private:
  friend class JsonParser;

  enum class Kind { Object, Array, String, Int, Bool, Float };

  Kind _kind;
  union Storage {
    bmin::Map<String, Json> object;
    bmin::DynArray<Json> array;
    String string;
    std::int64_t integer;
    bool boolean;
    double floating;

    Storage() {}
    ~Storage() {}
  } _storage;

  void destroy();
  void copyFrom(const Json& other);
  void swap(Json& other) noexcept;
  void resetToArray();
  void pushArrayValue(Json value);
  static const Json& emptySentinel();
};
