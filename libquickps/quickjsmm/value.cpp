#include <libquickps/quickjsmm/value.hpp>

namespace quickps {
namespace quickjs {

Value Value::GetNull() {
  JSValueUnion zero;
  JSValue null;
  zero.int32 = 0;
  null.u = zero;
  null.tag = JS_TAG_NULL;

  return Value(null);
}

Value Value::GetUndefined() {
  JSValueUnion zero;
  JSValue undefined;
  zero.int32 = 0;
  undefined.u = zero;
  undefined.tag = JS_TAG_UNDEFINED;

  return Value(undefined);
}

bool Value::operator==(const Value &other) const {
  if (JS_VALUE_IS_BOTH_INT(cobj_, other.cobj_)) {
    return JS_VALUE_GET_INT(cobj_) == JS_VALUE_GET_INT(other.cobj_);
  }

  if (JS_VALUE_IS_BOTH_FLOAT(cobj_, other.cobj_)) {
    return JS_VALUE_GET_FLOAT64(cobj_) == JS_VALUE_GET_FLOAT64(other.cobj_);
  }

  int64_t tag = JS_VALUE_GET_TAG(cobj_);

  if (tag != JS_VALUE_GET_TAG(other.cobj_))
    return false;

  if (tag < 0) {
    return JS_VALUE_GET_PTR(cobj_) == JS_VALUE_GET_PTR(other.cobj_);
  }

  return JS_VALUE_GET_BOOL(cobj_) == JS_VALUE_GET_BOOL(other.cobj_);
}

bool Value::IsException() { return JS_IsException(cobj_); }

bool Value::IsUndefined() {
  int64_t tag = JS_VALUE_GET_TAG(cobj_);
  return JS_TAG_UNDEFINED == tag;
}

JSValue &Value::cobj() { return cobj_; }

ValueIterator::reference ValueIter::operator*() { return current_.value(); }

ValueIterator::pointer ValueIter::operator->() { return &current_.value(); }

ValueIter &ValueIter::operator++() {
  count_++;
  current_.emplace(js_values_[count_]);
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
  return !(js_values_ == other.js_values_ && count_ == other.count_);
  //  return !(*this == other);
}

ValueIter ArgumentList::begin() { return ValueIter(argc_, js_values_); }

ValueIter ArgumentList::end() { return ValueIter(argc_, js_values_, argc_); }

RcValue::RcValue(ContextProvider &ctx, JSValue value)
    : ctx_(ctx), cobj_(std::make_optional<Value>(value)) {}

RcValue::RcValue(ContextProvider &ctx, Value value)
    : ctx_(ctx), cobj_(std::make_optional<Value>(value)) {}

RcValue::~RcValue() {
  if (cobj_) {
    JS_FreeValue(ctx_.cobj(), cobj());
    cobj_.reset();
  }
}

RcValue::RcValue(RcValue &&other) : ctx_(other.ctx_), cobj_() {
  cobj_.swap(other.cobj_);
}

bool RcValue::IsException() { return cobj_.value().IsException(); }

RcValue RcValue::Get(const char *prop) {
  Value property(JS_GetPropertyStr(ctx_.cobj(), cobj(), prop));

  if (property.IsException())
    throw Exception();

  if (property.IsUndefined())
    throw Exception();

  return RcValue(ctx_, property.cobj());
}

void RcValue::Set(const char *prop, Value v) {
  if (JS_SetPropertyStr(ctx_.cobj(), cobj(), prop, v.cobj()) < 1) {
    throw Exception();
  }
}

Value RcValue::Invoke(const std::string &method,
                      const std::vector<Value> &args) {
  JSAtom js_method = JS_NewAtomLen(ctx_.cobj(), method.c_str(), method.size());
  std::vector<JSValue> js_args({});
  for (Value v : args)
    js_args.push_back(v.cobj());

  Value ret(JS_Invoke(ctx_.cobj(), cobj(), js_method, js_args.size(),
                      js_args.data()));
  JS_FreeAtom(ctx_.cobj(), js_method);
  return ret;
}

Value RcValue::Call(const std::vector<Value> &args) {
  std::vector<JSValue> js_args({});
  for (Value v : args)
    js_args.push_back(v.cobj());

  Value ret(JS_Call(ctx_.cobj(), cobj(), Value::GetNull().cobj(),
                    js_args.size(), js_args.data()));
  return ret;
}

Value RcValue::value() { return cobj_.value(); }
JSValue RcValue::cobj() { return cobj_.value().cobj(); }

Value RcValue::Take() {
  Value tmp = cobj_.value();
  cobj_.reset();
  return tmp;
}

RcString::RcString(ContextProvider &ctx, const char *cobj)
    : ctx_(ctx), cobj_(std::make_optional<const char *>(cobj)) {}

RcString::~RcString() {
  if (cobj_) {
    JS_FreeCString(ctx_.cobj(), cobj());
  }
}

const char *RcString::cobj() { return cobj_.value(); }

const char *RcString::Take() {
  const char *tmp = cobj_.value();
  cobj_.reset();
  return tmp;
}

} // namespace quickjs
} // namespace quickps
