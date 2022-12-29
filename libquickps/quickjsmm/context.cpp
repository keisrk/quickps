#include <libquickps/quickjsmm/context.hpp>
#include <utility>

namespace quickps {
namespace quickjs {

JSContext *Context::cobj() { return cobj_; }
Value Context::Get(bool val) { return WrapValue(JS_NewBool(cobj_, val)); }
Value Context::Get(int val) { return WrapValue(JS_NewInt32(cobj_, val)); }
Value Context::Get(double val) { return WrapValue(JS_NewFloat64(cobj_, val)); }
Value Context::Get(const char *val) {
  return WrapValue(JS_NewString(cobj_, val));
}

RcValue Context::GetObject(const std::unordered_map<std::string, Value> &val) {
  auto obj = WrapRcValue(JS_NewObject(cobj()));

  for (auto kv : val) {
    if (int ret = JS_SetPropertyStr(cobj(), obj.cobj(), kv.first.c_str(),
                                    kv.second.cobj());
        1 != ret) {
      throw Exception();
    }
  }

  return obj;
}

RcValue Context::GetGlobalThis() {
  RcValue global(static_cast<ContextProvider &>(*this),
                 JS_GetGlobalObject(cobj_));

  if (global.IsException())
    throw Exception();

  return global;
}

void Context::Set(bool &ref, Value val) {
  ref = JS_ToBool(cobj_, val.cobj()) == 1;
}

void Context::Set(int &ref, Value val) {
  int *intermediate = &ref;
  JS_ToInt32(cobj_, intermediate, val.cobj());
}

void Context::Set(double &ref, Value val) {
  double *intermediate = &ref;
  JS_ToFloat64(cobj_, intermediate, val.cobj());
}

void Context::Set(std::string &ref, Value val) {
  auto js_str = JS_ToString(cobj_, val.cobj());
  ref.assign(JS_ToCString(cobj_, js_str));
}

Value Context::WrapValue(JSValue v) {
  Value value(v);

  if (value.IsException()) {
    throw Exception();
  }

  return value;
}

RcValue Context::WrapRcValue(JSValue v) {
  RcValue value((ContextProvider &)*this, v);

  if (value.IsException()) {
    throw Exception();
  }

  return value;
}

void ContextDeleter::operator()(Context *ctx) const {
  JS_FreeContext(ctx->cobj_);
  delete ctx;
}

std::optional<JSRuntime *> Runtime::cobj_;
std::unordered_map<std::size_t, JSClassID> Runtime::class_registry_ = {};

void Runtime::Init() {
  Runtime::cobj_ = JS_NewRuntime();
  Runtime::class_registry_.clear();
}

void Runtime::Terminate() {
  JS_FreeRuntime(Runtime::cobj_.value());
  Runtime::cobj_.reset();
  Runtime::class_registry_.clear();
}

Runtime Runtime::GetInstance() {
  if (!Runtime::cobj_) {
    throw Exception();
  }

  return Runtime();
}

const std::unique_ptr<Context, ContextDeleter> Runtime::CreateContext() {
  auto rt = Runtime::GetInstance();
  JSContext *js_context = JS_NewContext(rt.cobj_.value());
  return std::unique_ptr<Context, ContextDeleter>(new Context(js_context),
                                                  ContextDeleter());
}

} // namespace quickjs
} // namespace quickps

std::ostream &operator<<(std::ostream &os,
                         const quickps::quickjs::ContextProvider &ctx) {
  auto *ctx_cobj = const_cast<quickps::quickjs::ContextProvider &>(ctx).cobj();
  JSValue error = JS_GetException(ctx_cobj);

  auto str = JS_ToCString(ctx_cobj, error);
  if (str) {
    os << str << "\n";
    JS_FreeCString(ctx_cobj, str);
  } else {
    os << "[exception]\n";
  }

  if (!JS_IsError(ctx_cobj, error))
    return os;

  JSValue val = JS_GetPropertyStr(ctx_cobj, error, "stack");

  if (!JS_IsUndefined(val)) {
    auto str2 = JS_ToCString(ctx_cobj, val);
    if (str2) {
      os << str2;
      JS_FreeCString(ctx_cobj, str2);
    } else {
      os << "[exception]\n";
    }
  }

  JS_FreeValue(ctx_cobj, val);
  JS_FreeValue(ctx_cobj, error);

  return os;
}
