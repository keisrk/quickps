#include <gtest/gtest.h>
#include <paperjs/paper.h>

#include <libquickps/quickps.hpp>

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

TEST_F(E2eTest, ExportSvg) {
  auto scope = qps_.CreatePaperScope();
  auto canvas = scope.Inject().CreateCanvas(640, 480);
  auto engine = scope.SetUp(canvas);
  auto exe = engine.Compile(kSource);
  engine.Execute(exe);

  auto svg = engine.ExportSVG();
  EXPECT_NE(svg.find("<svg"), std::string::npos);
  EXPECT_NE(svg.find("xmlns=\"http://www.w3.org/2000/svg\""),
            std::string::npos);
  EXPECT_NE(svg.find("</svg>"), std::string::npos);
  EXPECT_NE(svg.find("<path"), std::string::npos);
}

TEST_F(E2eTest, PointTextFillAndStroke) {
  const char *kTextSource = R"(
    var text = new PointText({
        point: [30, 100],
        content: 'QuickPS',
        fontFamily: 'ABeeZee',
        fontWeight: 'bold',
        fontSize: 48,
        fillColor: '#ffffff',
        strokeColor: '#ff0000',
        strokeWidth: 4,
        strokeJoin: 'round',
        paintOrder: 'stroke'
    });
  )";

  auto scope = qps_.CreatePaperScope();
  auto canvas = scope.Inject().CreateCanvas(300, 150);
  auto *canvas_ptr = qps_.ctx().GetOpaque<canvas::Canvas>(canvas.value);
  auto engine = scope.SetUp(canvas);
  auto exe = engine.Compile(kTextSource);
  engine.Execute(exe);

  BLImageData data{};
  ASSERT_EQ(canvas_ptr->Impl().get_data(&data), BL_SUCCESS);
  const auto *pixels = static_cast<const uint32_t *>(data.pixel_data);
  const auto stride = data.stride / sizeof(uint32_t);

  size_t red_stroke_pixels = 0;
  size_t white_fill_pixels = 0;
  for (int y = 0; y < 150; ++y) {
    for (int x = 0; x < 300; ++x) {
      uint32_t px = pixels[y * stride + x];
      uint32_t r = (px >> 16) & 0xFFU;
      uint32_t g = (px >> 8) & 0xFFU;
      uint32_t b = px & 0xFFU;
      if (r > 200 && g < 30 && b < 30) {
        ++red_stroke_pixels;
      } else if (r > 200 && g > 200 && b > 200) {
        ++white_fill_pixels;
      }
    }
  }
  EXPECT_GT(red_stroke_pixels, 50U);
  EXPECT_GT(white_fill_pixels, 50U);
}

}  // namespace
