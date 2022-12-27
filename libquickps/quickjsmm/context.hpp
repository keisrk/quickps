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
class EsModule;

class Runtime final {
public:
  static void Init();
  static void Terminate();
  static Runtime GetInstance();
  static const std::unique_ptr<Context, ContextDeleter> CreateContext();

  template <typename T> static void Dtor(T *ptr) {
    auto rt = Runtime::GetInstance();
    js_free_rt(rt.js_runtime_.value(), ptr);
  }

  template <typename T>
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

  template <typename T, typename... Ts>
  const std::unique_ptr<T, OpaqueDeleter<T>> Ctor(Ts... args) {
    void *buf = js_mallocz_rt(js_runtime_.value(), sizeof(T));

    if (!buf)
      throw Exception();

    T *ptr = new (buf) T(args...);
    return std::unique_ptr<T, OpaqueDeleter<T>>(ptr, Dtor<T>);
  }

  template <typename T> T *GetOpaque(Value value) {
    auto js_point_class_id = ClassId<T>();
    void *s = JS_GetOpaque(value.value(), js_point_class_id);
    return static_cast<T *>(s);
  }

  template <typename T> void Finalize(Value value) {
    T *ptr = GetOpaque<T>(value);
    Runtime::Dtor<T>(ptr);
  }

  template <typename T> JSClassDef DefineClass(const char *name) {
    JSClassDef def;
    def.class_name = name;
    def.finalizer = [](auto, auto js_val) {
      Value v(js_val);
      Runtime::GetInstance().Finalize<T>(v);
    };

    JS_NewClass(js_runtime_.value(), ClassId<T>(), &def);
    return def;
  }

private:
  static std::optional<JSRuntime *> js_runtime_;
  static std::unordered_map<std::size_t, JSClassID> class_registry_;
  Runtime() {}
};

template <typename T> using OpaqueDeleter = Runtime::OpaqueDeleter<T>;

class Context final : public ContextProvider {
public:
  constexpr Context(JSContext *ctx) : js_context_(ctx) {}
  Context(const Context &) = delete;
  Context &operator=(const Context &) = delete;
  JSContext *GetInstance();
  Value Get(bool val);
  Value Get(int val);
  Value Get(double val);
  Value Get(const char *val);
  void Set(bool &ref, Value val);
  void Set(int &ref, Value val);
  void Set(double &ref, Value val);
  void Set(std::string &ref, Value val);
  void GetProperty();
  void FreeValue(JSValue value);
  void FreeString(const char *str);

  template <typename T> Value LoadModule(const T &buf) {
    static_assert(std::tuple_size<T>::value != 0, "Empty array");

    JSValue result =
        JS_ReadObject(js_context_, &buf.data()[0], std::tuple_size<T>::value,
                      JS_READ_OBJ_BYTECODE);

    return WrapValue(result);
  }

  template <typename T> T *GetOpaque(Value value) {
    auto js_point_class_id = Runtime::GetInstance().ClassId<T>();
    void *s = JS_GetOpaque2(GetInstance(), value.value(), js_point_class_id);
    return static_cast<T *>(s);
  }

  template <typename T>
  Value CreateOpaque(std::unique_ptr<T, OpaqueDeleter<T>> ptr,
                     Value new_target) {
    // // FIXME: Free proto and obj.
    auto js_point_class_id = Runtime::GetInstance().ClassId<T>();
    auto proto =
        JS_GetPropertyStr(GetInstance(), new_target.value(), "prototype");
    auto obj = JS_NewObjectProtoClass(GetInstance(), proto, js_point_class_id);
    JS_SetOpaque(obj, ptr.release());
    return WrapValue(obj);
  }

  friend class ContextDeleter;

private:
  JSContext *js_context_;

  Value WrapValue(JSValue v);
};

class ContextDeleter final {
public:
  void operator()(Context *ctx) const;
};

} // namespace quickjs
} // namespace quickps

#endif // QUICKPS_QUICKJSMM_CONTEXT_H_
