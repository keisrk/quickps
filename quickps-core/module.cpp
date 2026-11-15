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

    // 2. Compile the embedded paper-core JS source using the host qjs runtime
    auto src = paper::GetSource();
    JSValue mod_val =
        JS_Eval(ctx, src.data(), src.size(), module_name,
                JS_EVAL_TYPE_MODULE | JS_EVAL_FLAG_COMPILE_ONLY);
    if (JS_IsException(mod_val)) {
      JSValue exc = JS_GetException(ctx);
      const char *str = JS_ToCString(ctx, exc);
      fprintf(stderr, "JS_Eval exception: %s\n", str ? str : "unknown");
      JS_FreeCString(ctx, str);
      JS_FreeValue(ctx, exc);
      return nullptr;
    }

    auto *m = static_cast<JSModuleDef *>(JS_VALUE_GET_PTR(mod_val));
    JS_FreeValue(ctx, mod_val);
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
