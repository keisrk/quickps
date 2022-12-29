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

  EntryBuilder &DefType(uint8_t def_type) {
    entry_.def_type = def_type;
    return *this;
  }

  EntryBuilder &Name(const char *name) {
    entry_.name = name;
    return *this;
  }

  template <Fun F> EntryBuilder &Func(size_t argc) {
    static_assert(std::is_same<C, nullptr_t>::value);
    Descriptor(JS_PROP_WRITABLE | JS_PROP_CONFIGURABLE);
    DefType(JS_DEF_CFUNC);

    static JSCFunction *generic = [](auto *js_ctx, auto this_val, int argc,
                                     auto *argv) {
      Context ctx(js_ctx);
      Value me(this_val);
      ArgumentList args(argc, argv);
      return F(ctx, me, args.begin(), args.end()).cobj();
    };

    entry_.u.func.length = argc;
    entry_.u.func.cproto = JS_CFUNC_generic;
    entry_.u.func.cfunc.generic = generic;
    return *this;
  }

  EntryBuilder &Object() { return *this; }

  Entry Build() { return Entry(entry_); }

  template <Get<C> G> EntryBuilder &Getter() {
    static_assert(!std::is_same<C, nullptr_t>::value);
    Descriptor(JS_PROP_CONFIGURABLE);
    DefType(JS_DEF_CGETSET);

    static auto getter = [](auto *js_ctx, auto js_this_val) {
      Context ctx(js_ctx);
      Value this_val(js_this_val);
      C *ptr = ctx.GetOpaque<C>(this_val);
      return G(ctx, ptr).cobj();
    };

    entry_.u.getset.get.getter = getter;

    return *this;
  }

  template <Set<C> S> EntryBuilder &Setter() {
    static_assert(!std::is_same<C, nullptr_t>::value);
    Descriptor(JS_PROP_CONFIGURABLE);
    DefType(JS_DEF_CGETSET);

    static auto setter = [](auto *js_ctx, auto js_this_val, auto js_val) {
      Context ctx(js_ctx);
      Value this_val(js_this_val);
      C *ptr = ctx.GetOpaque<C>(this_val);
      Value v(js_val);
      S(ctx, ptr, v);
      return js_val;
    };

    entry_.u.getset.set.setter = setter;

    return *this;
  }

  template <MethodFun<C> M> EntryBuilder &Method(size_t argc) {
    static_assert(!std::is_same<C, nullptr_t>::value);
    Descriptor(JS_PROP_WRITABLE | JS_PROP_CONFIGURABLE);
    DefType(JS_DEF_CFUNC);

    static JSCFunction *generic = [](auto *js_ctx, auto js_this_val, int argc,
                                     auto *argv) {
      Context ctx(js_ctx);
      Value this_val(js_this_val);
      ArgumentList args(argc, argv);
      C *ptr = ctx.GetOpaque<C>(this_val);
      return M(ctx, ptr, args.begin(), args.end()).cobj();
    };

    entry_.u.func.length = argc;
    entry_.u.func.cproto = JS_CFUNC_generic;
    entry_.u.func.cfunc.generic = generic;

    return *this;
  }

private:
  JSCFunctionListEntry entry_;
};

template <class C> class ClassBuilder final {
public:
  ClassBuilder(const char *name) : name_(name), def_(), entries_() {
    static auto finalizer = [](auto, auto js_val) {
      Value v(js_val);
      Runtime::GetInstance().Finalize<C>(v);
    };

    def_.class_name = name;
    def_.finalizer = finalizer;
  }

  template <Ctor<C> CTOR> ClassBuilder &Constructor(size_t argc) {
    static auto ctor = [](auto *js_ctx, auto js_base_val, int argc,
                          auto *argv) {
      Context ctx(js_ctx);
      Value base_val(js_base_val);
      ArgumentList args(argc, argv);
      auto ptr = CTOR(ctx, args.begin(), args.end());
      return ctx.CreateOpaque<C>(std::move(ptr), base_val).cobj();
    };

    ctor_argc_ = argc;
    ctor_ = ctor;

    return *this;
  }

  ClassBuilder &Proto(Entry entry) {
    entries_.push_back(entry.cobj());
    return *this;
  }

  Class Build() { return Class{def_, ctor_, ctor_argc_, entries_}; }

private:
  const char *name_;
  JSClassDef def_;
  JSCFunction *ctor_ = nullptr;
  size_t ctor_argc_ = 0;
  std::vector<JSCFunctionListEntry> entries_;
};

} // namespace quickjs
} // namespace quickps

#endif // QUICKPS_QUICKJSMM_ESM_H_
/*
typedef struct JSCFunctionListEntry {
    const char *name;
    uint8_t prop_flags;
    uint8_t def_type;
    int16_t magic;
    func | generic | ... u;
*/
