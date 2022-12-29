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

} // namespace
