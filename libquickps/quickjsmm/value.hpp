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
  static Value GetNull();
  static Value GetUndefined();

  constexpr Value(JSValue value) : cobj_(value) {}

  bool operator==(const Value &other) const;

  bool IsException();

  bool IsUndefined();

  JSValue &cobj();

  template <typename T> T Get(ContextProvider &ctx) {
    if constexpr (std::is_same<T, bool>::value) {
      return JS_ToBool(ctx.cobj(), cobj_) == 1;
    } else if constexpr (std::is_same<T, int>::value) {
      int tmp;
      JS_ToInt32(ctx.cobj(), &tmp, cobj_);
      return tmp;
    } else if constexpr (std::is_same<T, double>::value) {
      double tmp;
      JS_ToFloat64(ctx.cobj(), &tmp, cobj_);
      return tmp;
    } else if constexpr (std::is_same<T, std::string>::value) {
      auto c_str = JS_ToCString(ctx.cobj(), cobj_);
      std::string result(c_str);
      JS_FreeCString(ctx.cobj(), c_str);
      return result;
    } else {
      static_assert(!std::is_same<T, void>::value,
                    "Value type void is invalid.");
      static_assert(std::is_same<T, void>::value, "Unsupported value type.");
    }
  }

private:
  JSValue cobj_;
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
  ValueIter(size_t argc, JSValue *js_values, size_t count = 0)
      : argc_(argc), js_values_(js_values), count_(count), current_() {
    if (count_ < argc_) {
      current_.emplace(js_values_[count_]);
    }
  }

  size_t argc_;
  JSValue *js_values_;
  size_t count_;
  std::optional<Value> current_;
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
  RcValue(ContextProvider &ctx, Value value);
  ~RcValue();
  RcValue(const RcValue &) = delete;
  RcValue &operator=(const RcValue &) = delete;
  RcValue(RcValue &&);
  RcValue &operator=(RcValue &&) = default;
  bool IsException();
  RcValue Get(const char *prop);
  void Set(const char *prop, Value v);
  Value Invoke(const std::string &method, const std::vector<Value> &args);
  Value Call(const std::vector<Value> &args);
  Value Take();
  Value value();
  JSValue cobj();

private:
  ContextProvider &ctx_;
  std::optional<Value> cobj_;
};

class RcString final {
public:
  RcString(ContextProvider &ctx, const char *cobj);
  ~RcString();
  RcString(const RcString &) = delete;
  RcString &operator=(const RcString &) = delete;
  const char *cobj();
  const char *Take();

private:
  ContextProvider &ctx_;
  std::optional<const char *> cobj_;
};

} // namespace quickjs
} // namespace quickps

#endif // QUICKPS_QUICKJSMM_VALUE_H_
