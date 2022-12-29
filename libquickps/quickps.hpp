#ifndef QUICKPS_QUICKPS_H_
#define QUICKPS_QUICKPS_H_

#include <libquickps/canvas/context.hpp>

namespace quickps {

// A plain old data struct that wraps canvas implementation.
struct PaperCanvas {
  PaperCanvas(quickjs::Value value);
  quickjs::Value value;
};

struct PaperExecutable {
  PaperExecutable(quickjs::Value code);
  quickjs::Value code;
};

// Forward declarations.
class PaperScope;
class PaperScript;
class PaperCanvasProvider;

class QuickPs {
public:
  void Init();
  void Terminate();
  PaperScope CreatePaperScope();
  quickjs::Context &ctx();
  quickjs::Value canvas();
  quickjs::Value c2d();
  quickjs::Value create_canvas();

private:
  std::unique_ptr<quickjs::Context, quickjs::ContextDeleter> ctx_;
  std::optional<quickjs::RcValue> canvas_;
  std::optional<quickjs::RcValue> c2d_;
  std::optional<quickjs::RcValue> create_canvas_;

  void LoadContext();
  void LoadPaper();
  void LoadCanvasClass();
  void LoadContext2dClass();
  void FreeContext();
  void FreePaper();
  void FreeCanvasClass();
  void FreeContext2dClass();
};

class PaperScope {
public:
  PaperScope(QuickPs &qps);
  PaperCanvasProvider Inject();
  PaperScript SetUp(PaperCanvas &canvas);

private:
  QuickPs &qps_;
};

class PaperScript {
public:
  PaperScript(QuickPs &qps, quickjs::Value scope);
  PaperExecutable Compile(const char *source);
  void Execute(const PaperExecutable &exe);

private:
  QuickPs &qps_;
  quickjs::RcValue scope_;
};

class PaperCanvasProvider {
public:
  PaperCanvasProvider(QuickPs &qps);
  PaperCanvas CreateCanvas(int width, int height);

private:
  QuickPs &qps_;
};

} // namespace quickps

#endif // QUICKPS_QUICKPS_H_
