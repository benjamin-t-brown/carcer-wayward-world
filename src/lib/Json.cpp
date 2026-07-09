#include "lib/Json.h"

#include "bmin/StringStream.h"

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cstdlib>
#include <stdexcept>
#include <utility>

namespace {

struct EmptyJsonObjectIterators {
  bmin::Map<bmin::String, Json> map;
  bmin::Map<bmin::String, Json>::Iterator it;
  bmin::Map<bmin::String, Json>::Iterator end;

  EmptyJsonObjectIterators() : it(map.end()), end(map.end()) {}
};

EmptyJsonObjectIterators& emptyJsonObjectIterators() {
  static EmptyJsonObjectIterators empty;
  return empty;
}

}  // namespace

class JsonParser {
 public:
  JsonParser(const char* text, bool ignoreComments)
      : _cursor(text), _start(text), _ignoreComments(ignoreComments) {}

  Json parseDocument();

 private:
  const char* _cursor;
  const char* _start;
  bool _ignoreComments;

  size_t column() const;
  [[noreturn]] void throwError(const char* message) const;
  char peek() const;
  char take();
  void skipWsAndComments();
  void expect(char ch);
  Json parseValue();
  bool startsWith(const char* literal) const;
  Json parseObject();
  Json parseArray();
  Json parseStringValue();
  bmin::String parseString();
  Json parseNumberValue();
};

Json JsonParser::parseDocument() {
  skipWsAndComments();
  if (*_cursor == '\0') {
    throwError("unexpected end of input");
  }
  auto value = parseValue();
  skipWsAndComments();
  if (*_cursor != '\0') {
    throwError("unexpected trailing input");
  }
  return value;
}

size_t JsonParser::column() const {
  return static_cast<size_t>(_cursor - _start) + 1;
}

void JsonParser::throwError(const char* message) const {
  bmin::StringStream ss;
  ss << "parse error at column " << column() << ": " << message;
  throw Json::parse_error(ss.str());
}

char JsonParser::peek() const {
  return *_cursor;
}

char JsonParser::take() {
  return *_cursor ? *_cursor++ : '\0';
}

void JsonParser::skipWsAndComments() {
  while (true) {
    while (*_cursor != '\0' && std::isspace(static_cast<unsigned char>(*_cursor)) != 0) {
      ++_cursor;
    }
    if (!_ignoreComments) {
      return;
    }
    if (_cursor[0] == '/' && _cursor[1] == '/') {
      _cursor += 2;
      while (*_cursor != '\0' && *_cursor != '\n') {
        ++_cursor;
      }
      continue;
    }
    if (_cursor[0] == '/' && _cursor[1] == '*') {
      _cursor += 2;
      while (_cursor[0] != '\0' && !(_cursor[0] == '*' && _cursor[1] == '/')) {
        ++_cursor;
      }
      if (_cursor[0] == '\0') {
        throwError("unterminated block comment");
      }
      _cursor += 2;
      continue;
    }
    return;
  }
}

void JsonParser::expect(char ch) {
  skipWsAndComments();
  if (take() != ch) {
    throwError("expected token");
  }
}

Json JsonParser::parseValue() {
  skipWsAndComments();
  const char ch = peek();
  if (ch == '{') {
    return parseObject();
  }
  if (ch == '[') {
    return parseArray();
  }
  if (ch == '"') {
    return parseStringValue();
  }
    if (ch == '-' || std::isdigit(static_cast<unsigned char>(ch)) != 0) {
      return parseNumberValue();
    }
  if (startsWith("true")) {
    _cursor += 4;
    Json value;
    value = true;
    return value;
  }
  if (startsWith("false")) {
    _cursor += 5;
    Json value;
    value = false;
    return value;
  }
  if (startsWith("null")) {
    throwError("null values are not supported");
  }
  throwError("invalid value");
}

bool JsonParser::startsWith(const char* literal) const {
  for (size_t i = 0; literal[i] != '\0'; ++i) {
    if (_cursor[i] != literal[i]) {
      return false;
    }
  }
  return true;
}

Json JsonParser::parseObject() {
  expect('{');
  Json object;
  skipWsAndComments();
  if (peek() == '}') {
    take();
    return object;
  }
  while (true) {
    skipWsAndComments();
    if (peek() != '"') {
      throwError("expected string key");
    }
    const bmin::String key = parseString();
    expect(':');
    object[key.cStr()] = parseValue();
    skipWsAndComments();
    const char ch = take();
    if (ch == '}') {
      break;
    }
    if (ch != ',') {
      throwError("expected ',' or '}'");
    }
  }
  return object;
}

Json JsonParser::parseArray() {
  expect('[');
  Json arrayValue;
  arrayValue.resetToArray();
  skipWsAndComments();
  if (peek() == ']') {
    take();
    return arrayValue;
  }
  while (true) {
    arrayValue.pushArrayValue(parseValue());
    skipWsAndComments();
    const char ch = take();
    if (ch == ']') {
      break;
    }
    if (ch != ',') {
      throwError("expected ',' or ']'");
    }
  }
  return arrayValue;
}

Json JsonParser::parseStringValue() {
  Json value;
  value = parseString();
  return value;
}

bmin::String JsonParser::parseString() {
  expect('"');
  bmin::String result;
  while (true) {
    const char ch = take();
    if (ch == '\0') {
      throwError("unterminated string");
    }
    if (ch == '"') {
      break;
    }
    if (ch == '\\') {
      const char escaped = take();
      if (escaped == '\0') {
        throwError("invalid string escape");
      }
      switch (escaped) {
        case '"':
          result.pushBack('"');
          break;
        case '\\':
          result.pushBack('\\');
          break;
        case '/':
          result.pushBack('/');
          break;
        case 'b':
          result.pushBack('\b');
          break;
        case 'f':
          result.pushBack('\f');
          break;
        case 'n':
          result.pushBack('\n');
          break;
        case 'r':
          result.pushBack('\r');
          break;
        case 't':
          result.pushBack('\t');
          break;
        case 'u':
          throwError("unicode escapes are not supported");
        default:
          throwError("invalid string escape");
      }
      continue;
    }
    if (static_cast<unsigned char>(ch) < 0x20) {
      throwError("control character in string");
    }
    result.pushBack(ch);
  }
  return result;
}

Json JsonParser::parseNumberValue() {
  const char* begin = _cursor;
  if (peek() == '-') {
    ++_cursor;
  }
  if (peek() == '0') {
    ++_cursor;
    if (std::isdigit(static_cast<unsigned char>(peek())) != 0) {
      throwError("invalid number");
    }
  } else {
    if (std::isdigit(static_cast<unsigned char>(peek())) == 0) {
      throwError("invalid number");
    }
    while (std::isdigit(static_cast<unsigned char>(peek())) != 0) {
      ++_cursor;
    }
  }

  const bool isFloat = peek() == '.' || peek() == 'e' || peek() == 'E';
  if (isFloat) {
    if (peek() == '.') {
      ++_cursor;
      if (std::isdigit(static_cast<unsigned char>(peek())) == 0) {
        throwError("invalid number");
      }
      while (std::isdigit(static_cast<unsigned char>(peek())) != 0) {
        ++_cursor;
      }
    }
    if (peek() == 'e' || peek() == 'E') {
      ++_cursor;
      if (peek() == '+' || peek() == '-') {
        ++_cursor;
      }
      if (std::isdigit(static_cast<unsigned char>(peek())) == 0) {
        throwError("invalid number");
      }
      while (std::isdigit(static_cast<unsigned char>(peek())) != 0) {
        ++_cursor;
      }
    }
    errno = 0;
    char* end = nullptr;
    const double parsed = std::strtod(begin, &end);
    if (errno == ERANGE || end != _cursor) {
      throwError("number out of range");
    }
    Json value;
    value.destroy();
    value._kind = Json::Kind::Float;
    value._storage.floating = parsed;
    return value;
  }

  errno = 0;
  char* end = nullptr;
  const auto parsed = std::strtoll(begin, &end, 10);
  if (errno == ERANGE || end != _cursor) {
    throwError("integer out of range");
  }
  Json value;
  value = static_cast<int>(parsed);
  if (static_cast<std::int64_t>(static_cast<int>(parsed)) != parsed) {
    throwError("integer out of range");
  }
  return value;
}

Json::parse_error::parse_error(bmin::String message) : _message(std::move(message)) {}

const char* Json::parse_error::what() const noexcept {
  return _message.cStr();
}

Json::Json() : _kind(Kind::Object) {
  new (&_storage.object) bmin::Map<bmin::String, Json>();
}

Json::Json(const Json& other) : _kind(Kind::Object) {
  new (&_storage.object) bmin::Map<bmin::String, Json>();
  copyFrom(other);
}

Json::Json(Json&& other) noexcept : _kind(other._kind) {
  switch (_kind) {
    case Kind::Object:
      new (&_storage.object) bmin::Map<bmin::String, Json>(std::move(other._storage.object));
      break;
    case Kind::Array:
      new (&_storage.array) bmin::DynArray<Json>(std::move(other._storage.array));
      break;
    case Kind::String:
      new (&_storage.string) bmin::String(std::move(other._storage.string));
      break;
    case Kind::Int:
      _storage.integer = other._storage.integer;
      break;
    case Kind::Bool:
      _storage.boolean = other._storage.boolean;
      break;
    case Kind::Float:
      _storage.floating = other._storage.floating;
      break;
  }
  other._kind = Kind::Object;
  new (&other._storage.object) bmin::Map<bmin::String, Json>();
}

Json& Json::operator=(Json other) {
  swap(other);
  return *this;
}

Json::~Json() {
  destroy();
}

void Json::destroy() {
  switch (_kind) {
    case Kind::Object:
      _storage.object.~Map();
      break;
    case Kind::Array:
      _storage.array.~DynArray();
      break;
    case Kind::String:
      _storage.string.~String();
      break;
    case Kind::Int:
    case Kind::Bool:
    case Kind::Float:
      break;
  }
}

void Json::copyFrom(const Json& other) {
  destroy();
  _kind = other._kind;
  switch (_kind) {
    case Kind::Object:
      new (&_storage.object) bmin::Map<bmin::String, Json>(other._storage.object);
      break;
    case Kind::Array:
      new (&_storage.array) bmin::DynArray<Json>(other._storage.array);
      break;
    case Kind::String:
      new (&_storage.string) bmin::String(other._storage.string);
      break;
    case Kind::Int:
      _storage.integer = other._storage.integer;
      break;
    case Kind::Bool:
      _storage.boolean = other._storage.boolean;
      break;
    case Kind::Float:
      _storage.floating = other._storage.floating;
      break;
  }
}

void Json::swap(Json& other) noexcept {
  if (_kind == other._kind) {
    switch (_kind) {
      case Kind::Object:
        using std::swap;
        swap(_storage.object, other._storage.object);
        break;
      case Kind::Array:
        using std::swap;
        swap(_storage.array, other._storage.array);
        break;
      case Kind::String:
        using std::swap;
        swap(_storage.string, other._storage.string);
        break;
      case Kind::Int:
        using std::swap;
        swap(_storage.integer, other._storage.integer);
        break;
      case Kind::Bool:
        using std::swap;
        swap(_storage.boolean, other._storage.boolean);
        break;
      case Kind::Float:
        using std::swap;
        swap(_storage.floating, other._storage.floating);
        break;
    }
    return;
  }

  Json tmp(std::move(other));
  other.destroy();
  other._kind = _kind;
  switch (_kind) {
    case Kind::Object:
      new (&other._storage.object) bmin::Map<bmin::String, Json>(std::move(_storage.object));
      _storage.object.~Map();
      break;
    case Kind::Array:
      new (&other._storage.array) bmin::DynArray<Json>(std::move(_storage.array));
      _storage.array.~DynArray();
      break;
    case Kind::String:
      new (&other._storage.string) bmin::String(std::move(_storage.string));
      _storage.string.~String();
      break;
    case Kind::Int:
      other._storage.integer = _storage.integer;
      break;
    case Kind::Bool:
      other._storage.boolean = _storage.boolean;
      break;
    case Kind::Float:
      other._storage.floating = _storage.floating;
      break;
  }

  _kind = tmp._kind;
  switch (_kind) {
    case Kind::Object:
      new (&_storage.object) bmin::Map<bmin::String, Json>(std::move(tmp._storage.object));
      break;
    case Kind::Array:
      new (&_storage.array) bmin::DynArray<Json>(std::move(tmp._storage.array));
      break;
    case Kind::String:
      new (&_storage.string) bmin::String(std::move(tmp._storage.string));
      break;
    case Kind::Int:
      _storage.integer = tmp._storage.integer;
      break;
    case Kind::Bool:
      _storage.boolean = tmp._storage.boolean;
      break;
    case Kind::Float:
      _storage.floating = tmp._storage.floating;
      break;
  }

  tmp._kind = Kind::Object;
  new (&tmp._storage.object) bmin::Map<bmin::String, Json>();
}

void Json::resetToArray() {
  destroy();
  _kind = Kind::Array;
  new (&_storage.array) bmin::DynArray<Json>();
}

void Json::pushArrayValue(Json value) {
  _storage.array.pushBack(std::move(value));
}

const Json& Json::emptySentinel() {
  static Json empty;
  return empty;
}

Json& Json::operator=(bmin::String value) {
  if (_kind != Kind::String) {
    destroy();
    _kind = Kind::String;
    new (&_storage.string) bmin::String(std::move(value));
  } else {
    _storage.string = std::move(value);
  }
  return *this;
}

Json& Json::operator=(const char* value) {
  return *this = bmin::String(value);
}

Json& Json::operator=(int value) {
  if (_kind != Kind::Int) {
    destroy();
    _kind = Kind::Int;
    _storage.integer = value;
  } else {
    _storage.integer = value;
  }
  return *this;
}

Json& Json::operator=(bool value) {
  if (_kind != Kind::Bool) {
    destroy();
    _kind = Kind::Bool;
    _storage.boolean = value;
  } else {
    _storage.boolean = value;
  }
  return *this;
}

bool Json::is_object() const {
  return _kind == Kind::Object;
}

bool Json::is_array() const {
  return _kind == Kind::Array;
}

bool Json::is_string() const {
  return _kind == Kind::String;
}

bool Json::is_number_integer() const {
  return _kind == Kind::Int;
}

bool Json::is_boolean() const {
  return _kind == Kind::Bool;
}

const Json& Json::operator[](const char* key) const {
  if (!is_object()) {
    return emptySentinel();
  }
  const bmin::String lookupKey(key);
  auto& map = const_cast<bmin::Map<bmin::String, Json>&>(_storage.object);
  const auto it = map.find(lookupKey);
  if (it == map.end()) {
    return emptySentinel();
  }
  return it->value;
}

Json& Json::operator[](const char* key) {
  if (!is_object()) {
    destroy();
    _kind = Kind::Object;
    new (&_storage.object) bmin::Map<bmin::String, Json>();
  }
  return _storage.object[bmin::String(key)];
}

const Json& Json::operator[](int index) const {
  return (*this)[static_cast<size_t>(index)];
}

Json& Json::operator[](int index) {
  return (*this)[static_cast<size_t>(index)];
}

const Json& Json::operator[](size_t index) const {
  if (!is_array()) {
    throw std::runtime_error("Json value is not an array");
  }
  if (index >= _storage.array.size()) {
    throw std::runtime_error("Json array index out of range");
  }
  return _storage.array[index];
}

Json& Json::operator[](size_t index) {
  if (!is_array()) {
    throw std::runtime_error("Json value is not an array");
  }
  if (index >= _storage.array.size()) {
    throw std::runtime_error("Json array index out of range");
  }
  return _storage.array[index];
}

bool Json::contains(const char* key) const {
  if (!is_object()) {
    return false;
  }
  return _storage.object.contains(bmin::String(key));
}

template <>
bmin::String Json::get<bmin::String>() const {
  if (!is_string()) {
    throw std::runtime_error("Json value is not a string");
  }
  return _storage.string;
}

template <>
int Json::get<int>() const {
  if (!is_number_integer()) {
    throw std::runtime_error("Json value is not an integer");
  }
  return static_cast<int>(_storage.integer);
}

template <>
bool Json::get<bool>() const {
  if (!is_boolean()) {
    throw std::runtime_error("Json value is not a boolean");
  }
  return _storage.boolean;
}

bmin::String Json::value(const char* key, bmin::String defaultValue) const {
  if (!is_object() || !contains(key)) {
    return defaultValue;
  }
  const Json& entry = (*this)[key];
  if (!entry.is_string()) {
    return defaultValue;
  }
  return entry.get<bmin::String>();
}

int Json::value(const char* key, int defaultValue) const {
  if (!is_object() || !contains(key)) {
    return defaultValue;
  }
  const Json& entry = (*this)[key];
  if (!entry.is_number_integer()) {
    return defaultValue;
  }
  return entry.get<int>();
}

bool Json::value(const char* key, bool defaultValue) const {
  if (!is_object() || !contains(key)) {
    return defaultValue;
  }
  const Json& entry = (*this)[key];
  if (!entry.is_boolean()) {
    return defaultValue;
  }
  return entry.get<bool>();
}

size_t Json::size() const {
  if (!is_array()) {
    return 0;
  }
  return _storage.array.size();
}

const Json* Json::begin() const {
  if (!is_array()) {
    return &emptySentinel();
  }
  return _storage.array.data();
}

const Json* Json::end() const {
  if (!is_array()) {
    return &emptySentinel();
  }
  return _storage.array.data() + _storage.array.size();
}

Json::JsonObjectIterator::JsonObjectIterator()
    : _map(&emptyJsonObjectIterators().map),
      _it(emptyJsonObjectIterators().it),
      _end(emptyJsonObjectIterators().end),
      _valid(false) {}

Json::JsonObjectIterator::JsonObjectIterator(bmin::Map<bmin::String, Json>* map,
                                             bmin::Map<bmin::String, Json>::Iterator it,
                                             bmin::Map<bmin::String, Json>::Iterator endIt)
    : _map(map), _it(it), _end(endIt), _valid(true) {}

Json::JsonObjectItem Json::JsonObjectIterator::operator*() const {
  return JsonObjectItem{_it->key, _it->value};
}

Json::JsonObjectIterator& Json::JsonObjectIterator::operator++() {
  if (_valid && _map != nullptr && _it != _end) {
    ++_it;
  }
  return *this;
}

bool Json::JsonObjectIterator::operator==(const JsonObjectIterator& other) const {
  if (!_valid && !other._valid) {
    return true;
  }
  if (_valid != other._valid) {
    return false;
  }
  return _map == other._map && _it == other._it;
}

bool Json::JsonObjectIterator::operator!=(const JsonObjectIterator& other) const {
  return !(*this == other);
}

Json::JsonObjectItemRange::JsonObjectItemRange(const Json& json) : _json(json) {}

Json::JsonObjectIterator Json::JsonObjectItemRange::begin() const {
  if (!_json.is_object()) {
    return JsonObjectIterator{};
  }
  auto& map = const_cast<Json&>(_json)._storage.object;
  return JsonObjectIterator{&map, map.begin(), map.end()};
}

Json::JsonObjectIterator Json::JsonObjectItemRange::end() const {
  if (!_json.is_object()) {
    return JsonObjectIterator{};
  }
  auto& map = const_cast<Json&>(_json)._storage.object;
  return JsonObjectIterator{&map, map.end(), map.end()};
}

Json::JsonObjectItemRange Json::items() const {
  return JsonObjectItemRange{*this};
}

Json Json::parse(const char* text, void* /*callback*/, bool allowExceptions,
                 bool ignoreComments) {
  try {
    JsonParser parser(text, ignoreComments);
    return parser.parseDocument();
  } catch (const Json::parse_error&) {
    if (allowExceptions) {
      throw;
    }
    return Json{};
  } catch (...) {
    if (allowExceptions) {
      throw;
    }
    return Json{};
  }
}
