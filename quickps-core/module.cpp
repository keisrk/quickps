#include <libquickps/quickjsmm/quickjs.hpp>

#include <libquickps/canvas/context.hpp>
#include <libquickps/quickjsmm/context.hpp>
#include <paperjs/paper.h>

#if defined(__GNUC__) || defined(__clang__)
#define QPS_EXPORT __attribute__((visibility("default")))
#else
#define QPS_EXPORT
#endif

extern "C" {

#ifdef JS_SHARED_LIBRARY
#define JS_INIT_MODULE js_init_module
#else
#define JS_INIT_MODULE js_init_module_qps
#endif

QPS_EXPORT JSModuleDef *js_init_module_qps(JSContext *ctx, const char *module_name);

QPS_EXPORT JSModuleDef *js_init_module(JSContext *ctx, const char *module_name) {
  return js_init_module_qps(ctx, module_name);
}

QPS_EXPORT JSModuleDef *js_init_module_qps(JSContext *ctx,
                                           [[maybe_unused]] const char *module_name) {
  try {
    // Attach current QuickJS runtime to quickjsmm wrapper
    quickps::quickjs::Runtime::Attach(JS_GetRuntime(ctx));

    // 1. Ensure Blend2D Canvas classes and 'canvas' module are registered in ctx
    quickps::quickjs::Context ctx_wrapper(ctx);
    quickps::canvas::RegisterCanvasModule(ctx_wrapper);

    // 2. Load the embedded paper-core bytecode
    auto buf = paper::GetByteCode();
    JSValue mod_val =
        JS_ReadObject(ctx, buf.data(), buf.size(), JS_READ_OBJ_BYTECODE);
    if (JS_IsException(mod_val)) {
      JSValue exc = JS_GetException(ctx);
      const char *str = JS_ToCString(ctx, exc);
      fprintf(stderr, "JS_ReadObject exception: %s\n", str ? str : "unknown");
      JS_FreeCString(ctx, str);
      JS_FreeValue(ctx, exc);
      return nullptr;
    }

    // 3. Resolve module dependencies
    if (JS_ResolveModule(ctx, mod_val) < 0) {
      JSValue exc = JS_GetException(ctx);
      const char *str = JS_ToCString(ctx, exc);
      fprintf(stderr, "JS_ResolveModule exception: %s\n", str ? str : "unknown");
      JS_FreeCString(ctx, str);
      JS_FreeValue(ctx, exc);
      JS_FreeValue(ctx, mod_val);
      return nullptr;
    }

    JSModuleDef *m = static_cast<JSModuleDef *>(JS_VALUE_GET_PTR(mod_val));

    // 4. Evaluate the module function
    JSValue res = JS_EvalFunction(ctx, mod_val);
    if (JS_IsException(res)) {
      JSValue exc = JS_GetException(ctx);
      const char *str = JS_ToCString(ctx, exc);
      fprintf(stderr, "JS_EvalFunction exception: %s\n", str ? str : "unknown");
      JS_FreeCString(ctx, str);
      JS_FreeValue(ctx, exc);
      return nullptr;
    }
    JS_FreeValue(ctx, res);

    return m;
  } catch (const std::exception &e) {
    fprintf(stderr, "Caught C++ exception in js_init_module_qps: %s\n", e.what());
    JSValue exc = JS_GetException(ctx);
    const char *str = JS_ToCString(ctx, exc);
    if (str) {
      fprintf(stderr, "QuickJS exception: %s\n", str);
      JS_FreeCString(ctx, str);
    }
    JS_FreeValue(ctx, exc);
    return nullptr;
  }
}

}  // extern "C"
