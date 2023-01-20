#include <libquickps/quickps.hpp>
#include <paper.h>

namespace quickps {

PaperCanvas::PaperCanvas(quickjs::Value canvas) : value(canvas) {}

PaperExecutable::PaperExecutable(quickjs::Value source_code)
    : code(source_code) {}

void QuickPs::Init() {
  quickjs::Runtime::Init();
  LoadContext();
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
  const auto &buf = paper::GetByteCode();
  auto module = ctx_->LoadModule<paper::ByteCode>(buf);
  auto module_eval =
      quickjs::Value(JS_EvalFunction(ctx_->cobj(), module.cobj()));

  if (module_eval.IsException())
    throw quickjs::Exception();
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

void QuickPs::FreePaper() {}

void QuickPs::FreeContext() { ctx_.reset(); }

PaperScope QuickPs::CreatePaperScope() { return PaperScope(*this); }

quickjs::Context &QuickPs::ctx() { return *ctx_.get(); }

quickjs::Value QuickPs::canvas() { return canvas_.value().Take(); }

quickjs::Value QuickPs::c2d() { return c2d_.value().Take(); }

quickjs::Value QuickPs::create_canvas() {
  return create_canvas_.value().Take();
}

PaperScope::PaperScope(QuickPs &qps) : qps_(qps) {
  auto window = qps_.ctx().GetGlobalThis().Get("self").Get("window");
  window.Set("HTMLCanvasElement", qps.canvas());
}

PaperCanvasProvider PaperScope::Inject() {
  auto scope = qps_.ctx().GetGlobalThis().Get("paper").Get("PaperScope");
  auto input = qps_.ctx().GetObject({});
  input.Set("createCanvas", qps_.create_canvas());
  auto ret = scope.Invoke("inject", {input.value()});
  JS_FreeValue(qps_.ctx().cobj(), ret.cobj());

  return PaperCanvasProvider(qps_);
}

PaperScript PaperScope::SetUp(PaperCanvas &canvas) {
  auto scope = qps_.ctx().GetGlobalThis().Get("paper").Get("PaperScope");
  auto instance = quickjs::RcValue(
      qps_.ctx(),
      JS_CallConstructor(qps_.ctx().cobj(), scope.cobj(), 0, nullptr));

  if (instance.IsException())
    throw quickjs::Exception();

  auto ret = instance.Invoke("setup", {canvas.value});
  if (ret.IsException()) {
    JS_FreeValue(qps_.ctx().cobj(), ret.cobj());
    JS_FreeValue(qps_.ctx().cobj(), canvas.value.cobj());
    throw quickjs::Exception();
  } else {
    JS_FreeValue(qps_.ctx().cobj(), ret.cobj());
    JS_FreeValue(qps_.ctx().cobj(), canvas.value.cobj());
    return PaperScript(qps_, instance.Take());
  }
}

PaperScript::PaperScript(QuickPs &qps, quickjs::Value scope)
    : qps_(qps), scope_(qps.ctx(), scope) {}

PaperExecutable PaperScript::Compile(const char *source) {
  auto engine = qps_.ctx().GetGlobalThis().Get("paper").Get("PaperScript");
  auto ps = qps_.ctx().Get(source);
  auto compiled = quickjs::RcValue(qps_.ctx(), engine.Invoke("compile", {ps}));

  if (compiled.IsException())
    throw quickjs::Exception();

  return PaperExecutable(compiled.Get("code").Take());
}

PaperExecutable PaperScript::Compile(const std::string &source) {
  return PaperScript::Compile(source.c_str());
}

void PaperScript::Execute(const PaperExecutable &exe) {
  auto engine = qps_.ctx().GetGlobalThis().Get("paper").Get("PaperScript");
  {
    auto ret = engine.Invoke("execute", {exe.code, scope_.value()});

    if (ret.IsException()) {
      JS_FreeValue(qps_.ctx().cobj(), ret.cobj());
      throw quickjs::Exception();
    } else {
      JS_FreeValue(qps_.ctx().cobj(), ret.cobj());
    }
  }
  {
    auto view = scope_.Get("project").Get("view");
    auto ret = view.Invoke("update", {});

    if (ret.IsException()) {
      JS_FreeValue(qps_.ctx().cobj(), ret.cobj());
      throw quickjs::Exception();
    } else {
      JS_FreeValue(qps_.ctx().cobj(), ret.cobj());
    }
  }
}

PaperCanvasProvider::PaperCanvasProvider(QuickPs &qps) : qps_(qps) {}

PaperCanvas PaperCanvasProvider::CreateCanvas(int width, int height) {
  auto create_canvas =
      qps_.ctx().GetGlobalThis().Get("paper").Get("createCanvas");

  std::vector<quickjs::Value> args({
      qps_.ctx().Get(width).cobj(),
      qps_.ctx().Get(height).cobj(),
      qps_.ctx().Get("image").cobj(),
  });

  auto canvas = create_canvas.Call(args);

  if (canvas.IsException())
    throw quickjs::Exception();

  return PaperCanvas(canvas);
}

} // namespace quickps
