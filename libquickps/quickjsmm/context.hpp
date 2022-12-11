#ifndef QUICKPS_QUICKJSMM_CONTEXT_H_
#define QUICKPS_QUICKJSMM_CONTEXT_H_

#include <array>
#include <libquickps/quickjsmm/value.hpp>
#include <memory>
#include <string>
#include <type_traits>

namespace quickps {
namespace quickjs {

// Forward declarations.
class Context;
class ContextDeleter;
class EsModule;

class Runtime final {
public:
  static const Runtime &GetInstance();
  static const std::unique_ptr<Context, ContextDeleter> CreateContext();

  template <typename T> static void Dtor(T *ptr) {
    const auto &rt = Runtime::GetInstance();
    js_free_rt(rt.js_runtime_, ptr);
  }

  template <typename T> static JSClassID ClassId() {
    static JSClassID js_point_class_id = 0;
    if (js_point_class_id == 0)
      JS_NewClassID(&js_point_class_id);

    return js_point_class_id;
  }

  template <typename T> static T *GetOpaque(Value value) {
    auto js_point_class_id = Runtime::ClassId<T>();
    void *s = JS_GetOpaque(value.value(), js_point_class_id);
    return static_cast<T *>(s);
  }

  template <typename T> static void Finalize(Value value) {
    T *ptr = Runtime::GetOpaque<T>(value);
    Runtime::Dtor<T>(ptr);
  }

  template <typename T> static JSClassDef DefineClass(const char *name) {
    JSClassDef def;
    def.class_name = name;
    def.finalizer = [](auto, auto js_val) {
      Value v(js_val);
      Runtime::Finalize<T>(v);
    };
    return def;
  }

  template <typename T>
  using OpaqueDeleter = std::add_pointer_t<decltype(Dtor<T>)>;

  template <typename T, typename... Ts>
  static const std::unique_ptr<T, OpaqueDeleter<T>> Ctor(Ts... args) {
    const auto &rt = Runtime::GetInstance();
    void *buf = js_mallocz_rt(rt.js_runtime_, sizeof(T));

    if (!buf)
      throw Exception();

    T *ptr = new (buf) T(args...);
    return std::unique_ptr<T, OpaqueDeleter<T>>(ptr, Dtor<T>);
  }

  Runtime(const Runtime &) = delete;
  Runtime &operator=(const Runtime &) = delete;

private:
  constexpr Runtime(JSRuntime *rt);
  JSRuntime *js_runtime_;
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
    auto js_point_class_id = Runtime::ClassId<T>();
    void *s = JS_GetOpaque2(GetInstance(), value.value(), js_point_class_id);
    return static_cast<T *>(s);
  }

  template <typename T>
  Value CreateOpaque(std::unique_ptr<T, OpaqueDeleter<T>> ptr,
                     Value new_target) {
    // // FIXME: Free proto and obj.
    auto js_point_class_id = Runtime::ClassId<T>();
    auto proto =
        JS_GetPropertyStr(GetInstance(), new_target.value(), "prototype");
    auto obj = JS_NewObjectProtoClass(GetInstance(), proto, js_point_class_id);
    JS_SetOpaque(obj, ptr.release());
    return WrapValue(obj);
  }

  //  void RegisterModule(EsModule esm);

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
