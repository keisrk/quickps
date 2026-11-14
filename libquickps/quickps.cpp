#include <paperjs/paper.h>

#include <libquickps/canvas/image.hpp>
#include <libquickps/quickps.hpp>

namespace quickps {

PaperCanvas::PaperCanvas(quickjs::Value canvas) : value(canvas) {}

PaperExecutable::PaperExecutable(quickjs::Value source_code)
    : code(source_code) {}

void QuickPs::Init() {
  quickjs::Runtime::Init();
  LoadContext();
  canvas::RegisterCanvasModule(*ctx_);
  LoadPaper();
  LoadCanvasClass();
  LoadContext2dClass();
}

void QuickPs::Terminate() {
  FreeContext2dClass();
  FreeCanvasClass();
  FreePaper();
  FreeContext();
  quickjs::Runtime::Terminate();
}

void QuickPs::LoadContext() {
  auto ctx = quickjs::Runtime::CreateContext();
  ctx_.swap(ctx);
}

void QuickPs::LoadPaper() {
  auto buf = paper::GetByteCode();
  auto module = ctx_->LoadModule(buf);
  if (module.IsException()) {
    throw quickjs::Exception();
  }

  JSModuleDef *m = static_cast<JSModuleDef *>(module.cobj().u.ptr);
  auto module_eval =
      quickjs::RcValue(*ctx_, JS_EvalFunction(ctx_->cobj(), module.cobj()));

  if (module_eval.IsException()) {
    throw quickjs::Exception();
  }

  auto ns = quickjs::RcValue(*ctx_, JS_GetModuleNamespace(ctx_->cobj(), m));
  if (ns.IsException()) throw quickjs::Exception();

  auto paper = ns.Get("default");
  if (paper.IsException()) throw quickjs::Exception();

  paper_.emplace(std::move(paper));
}

void QuickPs::LoadCanvasClass() {
  const auto &spec = quickjs::GetClass<canvas::Canvas>();
  canvas_.emplace(ctx_->Register<canvas::Canvas>(spec));
  create_canvas_.emplace(ctx_->Register(canvas::Canvas::CreateCanvas()));
}

void QuickPs::LoadContext2dClass() {
  const auto &spec = quickjs::GetClass<canvas::Context>();
  c2d_.emplace(ctx_->Register<canvas::Context>(spec));
}

void QuickPs::FreeContext2dClass() {
  create_canvas_.reset();
  c2d_.reset();
}

void QuickPs::FreeCanvasClass() { canvas_.reset(); }

void QuickPs::FreePaper() { paper_.reset(); }

void QuickPs::FreeContext() { ctx_.reset(); }

PaperScope QuickPs::CreatePaperScope() { return PaperScope(*this); }

quickjs::Context &QuickPs::ctx() { return *ctx_.get(); }

quickjs::RcValue &QuickPs::paper() { return paper_.value(); }

quickjs::Value QuickPs::canvas() {
  return quickjs::Value(JS_DupValue(ctx_->cobj(), canvas_.value().cobj()));
}

quickjs::Value QuickPs::c2d() {
  return quickjs::Value(JS_DupValue(ctx_->cobj(), c2d_.value().cobj()));
}

quickjs::Value QuickPs::create_canvas() {
  return quickjs::Value(JS_DupValue(ctx_->cobj(), create_canvas_.value().cobj()));
}

void QuickPs::RegisterImage(std::string_view id,
                            const std::filesystem::path &path) {
  auto resolved_path =
      std::filesystem::exists(path) ? std::filesystem::canonical(path) : path;
  auto image_ctor = ctx_->GetGlobalThis().Get("self").Get("Image");
  auto js_img = quickjs::RcValue(
      *ctx_, JS_CallConstructor(ctx_->cobj(), image_ctor.cobj(), 0, nullptr));
  if (js_img.IsException()) {
    throw quickjs::Exception();
  }
  auto *img = ctx_->GetOpaque<canvas::Image>(js_img.value());
  if (!img || !img->LoadFromFile(resolved_path)) {
    throw std::runtime_error("Failed to load image file: " + path.string());
  }
  js_img.Set("id", ctx_->Get(std::string(id)));
  auto doc = ctx_->GetGlobalThis().Get("self").Get("document");
  auto ret =
      doc.Invoke("registerImage", {ctx_->Get(std::string(id)), js_img.value()});
  if (ret.IsException()) {
    throw quickjs::Exception();
  }
  JS_FreeValue(ctx_->cobj(), ret.cobj());
}

PaperScope::PaperScope(QuickPs &qps) : qps_(qps) {
  auto window = qps_.ctx().GetGlobalThis().Get("self").Get("window");
  window.Set("HTMLCanvasElement", qps.canvas());
}

PaperCanvasProvider PaperScope::Inject() {
  auto scope = qps_.paper().Get("PaperScope");
  auto input = qps_.ctx().GetObject({});
  input.Set("createCanvas", qps_.create_canvas());
  auto ret = scope.Invoke("inject", {input.value()});
  JS_FreeValue(qps_.ctx().cobj(), ret.cobj());

  return PaperCanvasProvider(qps_);
}

PaperCanvas PaperScope::DefineScope(const ScopeConfig &config) {
  for (const auto &[id, path] : config.images) {
    qps_.RegisterImage(id, path);
  }

  auto canvas = Inject().CreateCanvas(config.width, config.height);

  if (config.on_draw) {
    auto *canvas_ptr = qps_.ctx().GetOpaque<canvas::Canvas>(canvas.value);
    canvas_ptr->SetOnDraw(config.on_draw);

    JSCFunction *on_draw_trampoline = [](JSContext *ctx, JSValueConst, int argc,
                                         JSValueConst *argv) -> JSValue {
      if (argc < 1) return quickjs::Value::GetUndefined().cobj();
      try {
        quickjs::Context ctx_wrapper(ctx);
        auto *cvs =
            ctx_wrapper.GetOpaque<canvas::Canvas>(quickjs::Value(argv[0]));
        cvs->InvokeOnDraw();
        return quickjs::Value::GetUndefined().cobj();
      } catch (const std::exception &e) {
        return JS_ThrowInternalError(ctx, "%s", e.what());
      } catch (...) {
        return JS_ThrowInternalError(ctx, "onDraw callback failed");
      }
    };

    JSValue fn =
        JS_NewCFunction(qps_.ctx().cobj(), on_draw_trampoline, "_onDraw", 1);
    quickjs::RcValue rc_canvas(qps_.ctx(), canvas.value);
    rc_canvas.Set("_onDraw", quickjs::Value(fn));
    canvas.value = rc_canvas.Take();
  }

  return canvas;
}

PaperScript PaperScope::SetUp(PaperCanvas &canvas) {
  auto scope = qps_.paper().Get("PaperScope");
  auto instance = quickjs::RcValue(
      qps_.ctx(),
      JS_CallConstructor(qps_.ctx().cobj(), scope.cobj(), 0, nullptr));

  if (instance.IsException()) {
    JS_FreeValue(qps_.ctx().cobj(), canvas.value.cobj());
    throw quickjs::Exception();
  }

  auto ret = instance.Invoke("setup", {canvas.value});
  JS_FreeValue(qps_.ctx().cobj(), ret.cobj());
  JS_FreeValue(qps_.ctx().cobj(), canvas.value.cobj());
  if (ret.IsException()) {
    throw quickjs::Exception();
  }
  return PaperScript(qps_, instance.Take());
}

PaperScript::PaperScript(QuickPs &qps, quickjs::Value scope)
    : qps_(qps), scope_(qps.ctx(), scope) {}

PaperExecutable PaperScript::Compile(const char *source) {
  auto engine = qps_.paper().Get("PaperScript");
  auto ps = quickjs::RcValue(qps_.ctx(), qps_.ctx().Get(source));
  auto compiled =
      quickjs::RcValue(qps_.ctx(), engine.Invoke("compile", {ps.value()}));

  if (compiled.IsException()) throw quickjs::Exception();

  return PaperExecutable(compiled.Get("code").Take());
}

PaperExecutable PaperScript::Compile(const std::string &source) {
  return PaperScript::Compile(source.c_str());
}

void PaperScript::Execute(const PaperExecutable &exe) {
  auto engine = qps_.paper().Get("PaperScript");
  {
    auto compiled = qps_.ctx().GetObject({});
    compiled.Set("code", exe.code);
    auto ret = engine.Invoke("execute", {compiled.value(), scope_.value()});

    if (ret.IsException()) {
      JS_FreeValue(qps_.ctx().cobj(), ret.cobj());
      throw quickjs::Exception();
    } else {
      JS_FreeValue(qps_.ctx().cobj(), ret.cobj());
    }
  }
  {
    auto view = scope_.Get("project").Get("view");
    auto ret = view.Invoke("draw", {});

    if (ret.IsException()) {
      JS_FreeValue(qps_.ctx().cobj(), ret.cobj());
      throw quickjs::Exception();
    } else {
      JS_FreeValue(qps_.ctx().cobj(), ret.cobj());
    }
  }
}

std::string PaperScript::ExportSVG() {
  auto project = scope_.Get("project");
  auto options = qps_.ctx().GetObject({});
  options.Set("asString", qps_.ctx().Get(true));
  auto ret = quickjs::RcValue(qps_.ctx(),
                              project.Invoke("exportSVG", {options.value()}));

  if (ret.IsException()) {
    throw quickjs::Exception();
  }

  return ret.value().Get<std::string>(qps_.ctx());
}

PaperCanvasProvider::PaperCanvasProvider(QuickPs &qps) : qps_(qps) {}

PaperCanvas PaperCanvasProvider::CreateCanvas(int width, int height) {
  auto create_canvas = qps_.paper().Get("createCanvas");

  std::vector<quickjs::Value> args({
      qps_.ctx().Get(width).cobj(),
      qps_.ctx().Get(height).cobj(),
      qps_.ctx().Get("image").cobj(),
  });

  auto canvas = create_canvas.Call(args);

  if (canvas.IsException()) throw quickjs::Exception();

  return PaperCanvas(canvas);
}

}  // namespace quickps
