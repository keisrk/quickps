#include <libquickps/quickjsmm/esm.hpp>
#include <memory>
#include <type_traits>
#include <unordered_map>
#include <utility>

namespace quickps {
namespace quickjs {

EsModuleBuilder::EsModuleBuilder(const char *name) : name_(name) {}

EsModule EsModuleBuilder::Build() const { return EsModule(); }

EsModuleBuilder EsModule::Builder(const char *name) {
  return EsModuleBuilder(name);
}

const char *EsModule::GetName() { return "hello"; }

} // namespace quickjs
} // namespace quickps

/*
JS_CGETSET_MAGIC_DEF("x", js_point_get_xy, js_point_set_xy, 0)

Entry::Accessor<C, C::Accessor::x>(name, fgetter, fsetter)
Entry::Accessor<C, C::Accessor::y>(name, fgetter, fsetter)
{ name, JS_PROP_CONFIGURABLE, JS_DEF_CGETSET, 0, .u = { .getset = { .get = {
.getter = fgetter }, .set = { .setter = fsetter } } } }


JS_CFUNC_DEF("norm", 0, js_point_norm)

Entry::Method<C, C::Accessor::norm>(name, length, func1)
(name, length, func1)
{ name, JS_PROP_WRITABLE | JS_PROP_CONFIGURABLE, JS_DEF_CFUNC, 0, .u = { .func =
{ length, JS_CFUNC_generic, { .generic = func1 } } } }

JSClassDef <- name & finalizer
JS_CFUNC_DEF
JS_CGETSET_MAGIC_DEF
getter [](&ctx, &C me){ return JS_NewInt32(ctx, me->x); }
setter [](&ctx, &C me, v){ return JS_NewInt32(ctx, me->x = v); }


using FunPtr = std::add_pointer_t<JSValue(int arg)>;

static FunPtr foo(const FunPtr other) {
  static FunPtr yet_another = other;
  return [](int a) { return yet_another(a); };
}

template<FP>
void foo() {

}

typedef JSValue JSCFunction(JSContext *ctx, JSValueConst this_val, int argc,
JSValueConst *argv);

template <typename InputIterator>
using Fun = std::add_pointer_t<Value (&Context ctx, Value this, InputIterator
first, InputIterator last)>;

using Get = std::add_pointer_t<Value (&Context ctx)>;
using Set = std::add_pointer_t<Value (&Context ctx, Value v)>;

template <typename InputIterator>
using Method = std::add_pointer_t<Value (&Context ctx, InputIterator first,
InputIterator last)>;

template <Fun F>
JSValue js_function(JSContext js_ctx*, JSValueConst js_this, int argc,
JSValueConst *argv) { Context ctx(js_ctx); Value c_this(js_this); auto args;
  F(ctx, )
}

template <Get G>
JSValue js_getter(JSContext js_ctx*, JSValue js_this) {
  Context ctx(js_ctx);
  G(*this)(ctx, )
}
 */
