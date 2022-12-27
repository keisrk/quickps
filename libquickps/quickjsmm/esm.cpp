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

void EsModule::Register(Context &ctx) {
  static std::unordered_map<const char *, int> kRegistry = {};

  auto esm_init = [](JSContext *js_ctx, JSModuleDef *m) {
    const char *module_name =
        JS_AtomToCString(js_ctx, JS_GetModuleName(js_ctx, m));
    const int def = kRegistry[module_name];
    return def;
  };

  kRegistry[GetName()] = 42;
  JS_NewCModule(ctx.GetInstance(), GetName(), esm_init);
}

} // namespace quickjs
} // namespace quickps
