#include <gtest/gtest.h>
#include <libquickps/quickps.hpp>
#include <paper.h>

namespace {
using namespace quickps;

class E2eTest : public ::testing::Test {
protected:
  E2eTest() : qps_() {}
  void SetUp() override { qps_.Init(); }
  void TearDown() override { qps_.Terminate(); }

  QuickPs qps_;
};

const char *kSource = R"(
    var style = {
        fillColor: new Color(74, 21, 173),
        strokeColor: new Color(74, 21, 173),
        strokeWidth: 1.5
    };

    var first = new Path.Rectangle([50, 50], [250, 250]);
    first.style = style;

    var path = new Path();
    // Give the stroke a color
    path.strokeColor = new Color(74, 21, 173);
    var start = new Point(100, 100);
    // Move to start and draw a line from there
    path.moveTo(start);
    // Note the plus operator on Point objects.
    // PaperScript does that for us, and much more!
    path.lineTo(start + [ 100, -50 ]);
)";

TEST_F(E2eTest, AllTheWay) {
  auto scope = qps_.CreatePaperScope();
  auto canvas = scope.Inject().CreateCanvas(640, 480);
  auto engine = scope.SetUp(canvas);
  auto exe = engine.Compile(kSource);
  engine.Execute(exe);
  EXPECT_TRUE(true);

  auto *canvas_ptr = qps_.ctx().GetOpaque<canvas::Canvas>(canvas.value);
  std::filesystem::path output = "output.png";
  canvas_ptr->Write(output);
}

} // namespace
