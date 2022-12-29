#ifndef QUICKPS_QUICKJSMM_CONTEXT_H_
#define QUICKPS_QUICKJSMM_CONTEXT_H_

#include <array>
#include <libquickps/quickjsmm/value.hpp>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <unordered_map>

namespace quickps {
namespace quickjs {

// Forward declarations.
class Context;
class ContextDeleter;

class Runtime final {
public:
  static void Init();
  static void Terminate();
  static Runtime GetInstance();
  static const std::unique_ptr<Context, ContextDeleter> CreateContext();

  template <class T> static void Dtor(T *ptr) {
    auto rt = Runtime::GetInstance();
    js_free_rt(rt.cobj_.value(), ptr);
  }

  template <class T>
  using OpaqueDeleter = std::add_pointer_t<decltype(Dtor<T>)>;

  Runtime(const Runtime &) = delete;
  Runtime &operator=(const Runtime &) = delete;

  template <typename T> JSClassID ClassId() {
    size_t id = typeid(T).hash_code();

    if (class_registry_.find(id) == class_registry_.end()) {
      class_registry_.emplace(id, 0);
      JS_NewClassID(&class_registry_[id]);
    }

    return class_registry_[id];
  }

  template <class T, typename... Ts>
  const std::unique_ptr<T, OpaqueDeleter<T>> Ctor(Ts... args) {
    void *buf = js_mallocz_rt(cobj_.value(), sizeof(T));

    if (!buf)
      throw Exception();

    T *ptr = new (buf) T(args...);
    return std::unique_ptr<T, OpaqueDeleter<T>>(ptr, Dtor<T>);
  }

  template <typename T> T *GetOpaque(Value value) {
    auto js_point_class_id = ClassId<T>();
    void *s = JS_GetOpaque(value.cobj(), js_point_class_id);
    return static_cast<T *>(s);
  }

  template <class T> void Finalize(Value value) {
    T *ptr = GetOpaque<T>(value);
    Runtime::Dtor<T>(ptr);
  }

  template <typename T> void Register(const JSClassDef &def) {
    if (int status = JS_NewClass(cobj_.value(), ClassId<T>(), &def);
        status < 0) {
      throw Exception();
    }
  }

private:
  static std::optional<JSRuntime *> cobj_;
  static std::unordered_map<std::size_t, JSClassID> class_registry_;
  Runtime() {}
};

template <class T> using OpaqueDeleter = Runtime::OpaqueDeleter<T>;
template <class T> const Class &GetClass();
template <class T>
std::unique_ptr<T, OpaqueDeleter<T>> New(Context &ctx, ValueIter first,
                                         ValueIter last);

class Context final : public ContextProvider {
public:
  constexpr Context(JSContext *ctx) : cobj_(ctx) {}
  Context(const Context &) = delete;
  Context &operator=(const Context &) = delete;
  JSContext *cobj();
  Value Get(bool val);
  Value Get(int val);
  Value Get(double val);
  Value Get(const char *val);
  RcValue GetGlobalThis();
  RcValue GetObject(const std::unordered_map<std::string, Value> &val);
  void Set(bool &ref, Value val);
  void Set(int &ref, Value val);
  void Set(double &ref, Value val);
  void Set(std::string &ref, Value val);
  void GetProperty();

  template <typename T> Value LoadModule(const T &buf) {
    static_assert(std::tuple_size<T>::value != 0, "Empty array");

    JSValue result = JS_ReadObject(
        cobj_, &buf.data()[0], std::tuple_size<T>::value, JS_READ_OBJ_BYTECODE);

    return WrapValue(result);
  }

  template <typename T> RcValue Register(const Class &spec) {
    Runtime::GetInstance().Register<T>(spec.def);
    auto prototype = WrapValue(JS_NewObject(cobj()));
    JS_SetPropertyFunctionList(cobj(), prototype.cobj(), spec.prototype.data(),
                               spec.prototype.size());
    auto constructor =
        WrapRcValue(JS_NewCFunction2(cobj(), spec.ctor, spec.def.class_name,
                                     spec.ctor_argc, JS_CFUNC_constructor, 0));

    JS_SetConstructor(cobj(), constructor.cobj(), prototype.cobj());
    JS_SetClassProto(cobj(), Runtime::GetInstance().ClassId<T>(),
                     prototype.cobj());

    return constructor;
  }

  RcValue Register(Entry &spec) {
    auto function = WrapRcValue(JS_NewCFunction2(
        cobj(), spec.cobj().u.func.cfunc.generic, spec.cobj().name,
        spec.cobj().u.func.length, JS_CFUNC_generic, 0));
    return function;
  }

  template <typename T> T *GetOpaque(Value value) {
    auto js_point_class_id = Runtime::GetInstance().ClassId<T>();
    void *s = JS_GetOpaque2(cobj(), value.cobj(), js_point_class_id);

    if (!s) {
      throw Exception();
    }

    return static_cast<T *>(s);
  }

  template <typename T>
  Value CreateOpaque(std::unique_ptr<T, OpaqueDeleter<T>> ptr,
                     [[maybe_unused]] Value new_target) {
    auto js_point_class_id = Runtime::GetInstance().ClassId<T>();
    auto obj = JS_NewObjectClass(cobj(), js_point_class_id);
    JS_SetOpaque(obj, ptr.release());
    return WrapValue(obj);
  }

  friend class ContextDeleter;

private:
  JSContext *cobj_;

  Value WrapValue(JSValue v);
  RcValue WrapRcValue(JSValue v);
};

class ContextDeleter final {
public:
  void operator()(Context *ctx) const;
};

} // namespace quickjs
} // namespace quickps

std::ostream &operator<<(std::ostream &os,
                         const quickps::quickjs::ContextProvider &ctx);

#endif // QUICKPS_QUICKJSMM_CONTEXT_H_
