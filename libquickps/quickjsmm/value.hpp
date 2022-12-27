#ifndef QUICKPS_QUICKJSMM_VALUE_H_
#define QUICKPS_QUICKJSMM_VALUE_H_

#include <iterator>
#include <libquickps/quickjsmm/quickjs.hpp>
#include <optional>
#include <string>
#include <type_traits>

namespace quickps {
namespace quickjs {

class Value final {
public:
  constexpr Value(JSValue value) : js_value_(value) {}

  bool operator==(const Value &other) const;

  bool IsException();

  JSValue &value();

  template <typename T> T Get(ContextProvider &ctx) {
    if constexpr (std::is_same<T, bool>::value) {
      return JS_ToBool(ctx.GetInstance(), js_value_) == 1;
    } else if constexpr (std::is_same<T, int>::value) {
      int tmp;
      JS_ToInt32(ctx.GetInstance(), &tmp, js_value_);
      return tmp;
    } else if constexpr (std::is_same<T, double>::value) {
      double tmp;
      JS_ToFloat64(ctx.GetInstance(), &tmp, js_value_);
      return tmp;
    } else if constexpr (std::is_same<T, std::string>::value) {
      auto c_str = JS_ToCString(ctx.GetInstance(), js_value_);
      std::string result(c_str);
      JS_FreeCString(ctx.GetInstance(), c_str);
      return result;
    } else {
      static_assert(!std::is_same<T, void>::value,
                    "Value type void is invalid.");
      static_assert(std::is_same<T, void>::value, "Unsupported value type.");
    }
  }

private:
  JSValue js_value_;
};

using ValueIterator = std::iterator<std::input_iterator_tag, Value>;

class ValueIter : public ValueIterator {
public:
  reference operator*();
  pointer operator->();
  ValueIter &operator++();
  ValueIter operator++(int);
  bool operator==(const ValueIter &other) const;
  bool operator!=(const ValueIter &other) const;

  friend class ArgumentList;

private:
  constexpr ValueIter(size_t argc, JSValue *js_values, size_t count = 0)
      : argc_(argc), js_values_(js_values), count_(count),
        current_(js_values[count]) {}

  size_t argc_;
  JSValue *js_values_;
  size_t count_;
  Value current_;
};

class ArgumentList {
public:
  constexpr ArgumentList(int argc, JSValue *js_values)
      : argc_(argc), js_values_(js_values) {}

  ValueIter begin();
  ValueIter end();

private:
  int argc_;
  JSValue *js_values_;
};

class RcValue final {
public:
  RcValue(ContextProvider &ctx, JSValue value);
  ~RcValue();
  RcValue(const RcValue &) = delete;
  RcValue &operator=(const RcValue &) = delete;
  bool IsException();
  JSValue Get();
  JSValue Take();

private:
  ContextProvider &ctx_;
  std::optional<JSValue> js_value_;
};

class RcString final {
public:
  RcString(ContextProvider &ctx, const char *raw_string);
  ~RcString();
  RcString(const RcString &) = delete;
  RcString &operator=(const RcString &) = delete;
  const char *Get();
  const char *Take();

private:
  ContextProvider &ctx_;
  std::optional<const char *> raw_string_;
};

} // namespace quickjs
} // namespace quickps

#endif // QUICKPS_QUICKJSMM_VALUE_H_
