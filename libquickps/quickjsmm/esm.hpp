#ifndef QUICKPS_QUICKJSMM_ESM_H_
#define QUICKPS_QUICKJSMM_ESM_H_

#include <libquickps/quickjsmm/context.hpp>
#include <type_traits>
#include <vector>

namespace quickps {
namespace quickjs {

using Fun = std::add_pointer<Value(Context &ctx, Value this_val,
                                   ValueIter first, ValueIter last)>::type;

template <class C>
using Ctor = typename std::add_pointer<std::unique_ptr<C, OpaqueDeleter<C>>(
    Context &ctx, ValueIter first, ValueIter last)>::type;

template <class C>
using Get = typename std::add_pointer<Value(Context &ctx, C *instance)>::type;

template <class C>
using Set =
    typename std::add_pointer<void(Context &ctx, C *instance, Value v)>::type;

template <class C>
using MethodFun = typename std::add_pointer<Value(
    Context &ctx, C *instance, ValueIter first, ValueIter last)>::type;

template <class C = nullptr_t> class EntryBuilder final {
public:
  constexpr EntryBuilder() : entry_() {}

  EntryBuilder &Descriptor(uint8_t descriptor) {
    entry_.prop_flags = descriptor;
    return *this;
  }

  EntryBuilder &Name(const char *name) {
    entry_.name = name;
    return *this;
  }

  template <Fun F> EntryBuilder &Func(size_t argc) {
    static_assert(std::is_same<C, nullptr_t>::value);

    JSCFunction *generic = [](auto *js_ctx, auto this_val, int argc,
                              auto *argv) {
      Context ctx(js_ctx);
      Value me(this_val);
      ArgumentList args(argc, argv);
      return F(ctx, me, args.begin(), args.end()).value();
    };

    entry_.u.func.length = argc;
    entry_.u.func.cproto = JS_CFUNC_generic;
    entry_.u.func.cfunc.generic = generic;
    return *this;
  }

  EntryBuilder &Object() { return *this; }

  JSCFunctionListEntry Build() { return entry_; }

  template <Get<C> G> EntryBuilder &Getter() {
    static_assert(!std::is_same<C, nullptr_t>::value);

    auto getter = [](auto *js_ctx, auto js_this_val) {
      Context ctx(js_ctx);
      Value this_val(js_this_val);
      C *ptr = ctx.GetOpaque<C>(this_val);
      return G(ctx, ptr).value();
    };

    entry_.u.getset.get.getter = getter;

    return *this;
  }

  template <Set<C> S> EntryBuilder &Setter() {
    static_assert(!std::is_same<C, nullptr_t>::value);

    auto setter = [](auto *js_ctx, auto js_this_val, auto js_val) {
      Context ctx(js_ctx);
      Value this_val(js_this_val);
      C *ptr = ctx.GetOpaque<C>(this_val);
      Value v(js_val);
      S(ctx, ptr, v);
      return js_val;
    };

    entry_.u.getset.get.setter = setter;

    return *this;
  }

  template <MethodFun<C> M> EntryBuilder &Method(size_t argc) {
    static_assert(!std::is_same<C, nullptr_t>::value);

    JSCFunction *generic = [](auto *js_ctx, auto js_this_val, int argc,
                              auto *argv) {
      Context ctx(js_ctx);
      Value this_val(js_this_val);
      ArgumentList args(argc, argv);
      C *ptr = ctx.GetOpaque<C>(this_val);
      return M(ctx, ptr, args.begin(), args.end()).value();
    };

    entry_.u.func.length = argc;
    entry_.u.func.cproto = JS_CFUNC_generic;
    entry_.u.func.cfunc.generic = generic;

    return *this;
  }

private:
  JSCFunctionListEntry entry_;
};

template <class C> class ClassSpec final {
public:
  constexpr ClassSpec(const char *name)
      : def_(Runtime::GetInstance().DefineClass<C>(name)), entries_() {}

  template <Ctor<C> CTOR> ClassSpec &Constructor(size_t argc) {
    ctor_argc_ = argc;

    ctor_ = [](auto *js_ctx, auto js_base_val, int argc, auto *argv) {
      Context ctx(js_ctx);
      Value base_val(js_base_val);
      ArgumentList args(argc, argv);
      auto ptr = CTOR(ctx, args.begin(), args.end());
      return ctx.CreateOpaque<C>(std::move(ptr), base_val).value();
    };

    return *this;
  }

  ClassSpec &Proto(JSCFunctionListEntry entry) {
    entries_.push_back(entry);
    return *this;
  }

  Value Register(Context &ctx) {
    // FIXME: Free prototype.
    auto prototype = Value(JS_NewObject(ctx.GetInstance()));

    if (prototype.IsException())
      throw Exception();

    JS_SetPropertyFunctionList(ctx.GetInstance(), prototype.value(),
                               entries_.data(), entries_.size());
    auto class_def =
        Value(JS_NewCFunction2(ctx.GetInstance(), ctor_, def_.class_name,
                               ctor_argc_, JS_CFUNC_constructor, 0));

    if (class_def.IsException())
      throw Exception();

    JS_SetConstructor(ctx.GetInstance(), class_def.value(), prototype.value());
    JS_SetClassProto(ctx.GetInstance(), Runtime::GetInstance().ClassId<C>(),
                     prototype.value());

    return class_def;
  }

private:
  JSClassDef def_;
  JSCFunction *ctor_ = nullptr;
  size_t ctor_argc_ = 0;
  std::vector<JSCFunctionListEntry> entries_;
};

class EsModule;

class EsModuleBuilder final {
public:
  static int Init(JSContext *ctx, JSModuleDef *m);
  EsModuleBuilder(const char *name);
  const EsModuleBuilder &Function();

  template <typename C> EsModuleBuilder &Class() { return *this; }

  EsModule Build() const;

private:
  const char *name_;
};

class EsModule final {
public:
  static EsModuleBuilder Builder(const char *name);
  const char *GetName();
  void Register(Context &ctx);
};

} // namespace quickjs
} // namespace quickps

#endif // QUICKPS_QUICKJSMM_ESM_H_
