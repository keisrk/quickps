#include <libquickps/quickjsmm/context.hpp>
#include <utility>

namespace quickps {
namespace quickjs {

JSContext *Context::GetInstance() { return js_context_; }
Value Context::Get(bool val) { return WrapValue(JS_NewBool(js_context_, val)); }
Value Context::Get(int val) { return WrapValue(JS_NewInt32(js_context_, val)); }
Value Context::Get(double val) {
  return WrapValue(JS_NewFloat64(js_context_, val));
}
Value Context::Get(const char *val) {
  return WrapValue(JS_NewString(js_context_, val));
}

void Context::Set(bool &ref, Value val) {
  ref = JS_ToBool(js_context_, val.value()) == 1;
}

void Context::Set(int &ref, Value val) {
  int *intermediate = &ref;
  JS_ToInt32(js_context_, intermediate, val.value());
}

void Context::Set(double &ref, Value val) {
  double *intermediate = &ref;
  JS_ToFloat64(js_context_, intermediate, val.value());
}

void Context::Set(std::string &ref, Value val) {
  auto js_str = JS_ToString(js_context_, val.value());
  ref.assign(JS_ToCString(js_context_, js_str));
}

void Context::FreeValue(JSValue value) { JS_FreeValue(js_context_, value); }
void Context::FreeString(const char *str) { JS_FreeCString(js_context_, str); }

Value Context::WrapValue(JSValue v) {
  Value value(v);

  if (value.IsException()) {
    throw Exception();
  }

  return value;
}

void ContextDeleter::operator()(Context *ctx) const {
  JS_FreeContext(ctx->js_context_);
  delete ctx;
}

std::optional<JSRuntime *> Runtime::js_runtime_;
std::unordered_map<std::size_t, JSClassID> Runtime::class_registry_ = {};

void Runtime::Init() {
  Runtime::js_runtime_ = JS_NewRuntime();
  Runtime::class_registry_.clear();
}

void Runtime::Terminate() {
  JS_FreeRuntime(Runtime::js_runtime_.value());
  Runtime::js_runtime_.reset();
  Runtime::class_registry_.clear();
}

Runtime Runtime::GetInstance() {
  if (!Runtime::js_runtime_) {
    throw Exception();
  }

  return Runtime();
}

const std::unique_ptr<Context, ContextDeleter> Runtime::CreateContext() {
  auto rt = Runtime::GetInstance();
  JSContext *js_context = JS_NewContext(rt.js_runtime_.value());
  return std::unique_ptr<Context, ContextDeleter>(new Context(js_context),
                                                  ContextDeleter());
}

} // namespace quickjs
} // namespace quickps
