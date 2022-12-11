#include <libquickps/quickjsmm/value.hpp>

namespace quickps {
namespace quickjs {

bool Value::operator==(const Value &other) const {
  if (JS_VALUE_IS_BOTH_INT(js_value_, other.js_value_)) {
    return JS_VALUE_GET_INT(js_value_) == JS_VALUE_GET_INT(other.js_value_);
  }

  if (JS_VALUE_IS_BOTH_FLOAT(js_value_, other.js_value_)) {
    return JS_VALUE_GET_FLOAT64(js_value_) ==
           JS_VALUE_GET_FLOAT64(other.js_value_);
  }

  int64_t tag = JS_VALUE_GET_TAG(js_value_);

  if (tag != JS_VALUE_GET_TAG(other.js_value_))
    return false;

  if (tag < 0) {
    return JS_VALUE_GET_PTR(js_value_) == JS_VALUE_GET_PTR(other.js_value_);
  }

  return JS_VALUE_GET_BOOL(js_value_) == JS_VALUE_GET_BOOL(other.js_value_);
}

bool Value::IsException() { return JS_IsException(js_value_); }

JSValue &Value::value() { return js_value_; }

ValueIterator::reference ValueIter::operator*() { return current_; }

ValueIterator::pointer ValueIter::operator->() { return &current_; }

ValueIter &ValueIter::operator++() {
  count_++;
  current_ = Value(js_values_[count_]);
  return *this;
}

ValueIter ValueIter::operator++(int) {
  ValueIter tmp = *this;
  ++(*this);
  return tmp;
}

bool ValueIter::operator==(const ValueIter &other) const {
  return js_values_ == other.js_values_ && count_ == other.count_;
}

bool ValueIter::operator!=(const ValueIter &other) const {
  return !(*this == other);
}

ValueIter ArgumentList::begin() { return ValueIter(argc_, js_values_); }

ValueIter ArgumentList::end() { return ValueIter(argc_, js_values_, argc_); }

RcValue::RcValue(ContextProvider &ctx, JSValue value)
    : ctx_(ctx), js_value_(std::make_optional<JSValue>(value)) {}

RcValue::~RcValue() {
  if (js_value_) {
    JS_FreeValue(ctx_.GetInstance(), Get());
  }
}

bool RcValue::IsException() { return JS_IsException(js_value_.value()); }

JSValue RcValue::Get() { return js_value_.value(); }

JSValue RcValue::Take() {
  JSValue tmp = js_value_.value();
  js_value_.reset();
  return tmp;
}

RcString::RcString(ContextProvider &ctx, const char *raw_string)
    : ctx_(ctx), raw_string_(std::make_optional<const char *>(raw_string)) {}

RcString::~RcString() {
  if (raw_string_) {
    JS_FreeCString(ctx_.GetInstance(), Get());
  }
}

const char *RcString::Get() { return raw_string_.value(); }

const char *RcString::Take() {
  const char *tmp = raw_string_.value();
  raw_string_.reset();
  return tmp;
}

} // namespace quickjs
} // namespace quickps

// Value::~Value() { JS_FreeValue(ctx_.js_context_, js_value_); }
