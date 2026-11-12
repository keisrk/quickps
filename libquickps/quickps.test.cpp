#include <gtest/gtest.h>

#include <libquickps/quickps.hpp>

namespace {

using namespace quickps;

class QuickPsTest : public ::testing::Test {
 protected:
  QuickPsTest() : qps_() {}
  void SetUp() override { qps_.Init(); }
  void TearDown() override { qps_.Terminate(); }

  QuickPs qps_;
};

TEST_F(QuickPsTest, Init) {
  auto scope = qps_.CreatePaperScope();
  EXPECT_NE(&scope, nullptr);
  auto canvas = scope.Inject().CreateCanvas(640, 480);
  auto engine = scope.SetUp(canvas);
  EXPECT_NE(&engine, nullptr);

  auto canvas_ctor = qps_.ctx().GetGlobalThis().Get("self").Get("window").Get(
      "HTMLCanvasElement");

  auto is_instanceof_canvas = JS_IsInstanceOf(
      qps_.ctx().cobj(), canvas.value.cobj(), canvas_ctor.cobj());
  EXPECT_TRUE(is_instanceof_canvas);
}

TEST_F(QuickPsTest, DefineScope) {
  ScopeConfig config{
      .width = 800,
      .height = 600,
  };
  auto scope = qps_.CreatePaperScope();
  auto canvas = scope.DefineScope(config);
  auto engine = scope.SetUp(canvas);
  EXPECT_NE(&engine, nullptr);

  auto canvas_ctor = qps_.ctx().GetGlobalThis().Get("self").Get("window").Get(
      "HTMLCanvasElement");
  auto is_instanceof_canvas = JS_IsInstanceOf(
      qps_.ctx().cobj(), canvas.value.cobj(), canvas_ctor.cobj());
  EXPECT_TRUE(is_instanceof_canvas);
}

TEST_F(QuickPsTest, RegisterImageAndResolveViaRaster) {
  auto smiley_path = std::filesystem::path("examples/smiley.png");
  if (!std::filesystem::exists(smiley_path)) {
    if (const char *src_root = std::getenv("MESON_SOURCE_ROOT")) {
      smiley_path = std::filesystem::path(src_root) / "examples" / "smiley.png";
    } else {
      smiley_path = "../../examples/smiley.png";
    }
  }
  ASSERT_TRUE(std::filesystem::exists(smiley_path));

  qps_.RegisterImage("smiley", smiley_path);

  auto scope = qps_.CreatePaperScope();
  auto canvas = scope.Inject().CreateCanvas(400, 400);
  auto engine = scope.SetUp(canvas);

  const char *kScript = R"(
    var img = document.getElementById('smiley');
    if (!img || img.naturalWidth !== 200 || img.naturalHeight !== 200) {
      throw new Error('document.getElementById failed or dimensions mismatch');
    }
    var r1 = new Raster('smiley');
    if (r1.width !== 200 || r1.height !== 200) {
      throw new Error('Raster by id failed: ' + r1.width + 'x' + r1.height);
    }
    var r2 = new Raster(img);
    if (r2.width !== 200 || r2.height !== 200) {
      throw new Error('Raster by element failed: ' + r2.width + 'x' + r2.height);
    }
  )";

  try {
    auto exe = engine.Compile(kScript);
    engine.Execute(exe);
  } catch (const quickjs::Exception &) {
    std::cerr << "QuickJS error: " << qps_.ctx() << std::endl;
    FAIL() << "QuickJS exception thrown";
  } catch (const std::exception &e) {
    FAIL() << "C++ exception: " << e.what();
  }
}

}  // namespace
