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

TEST_F(E2eTest, RasterCreationAndPixelManipulation) {
  const char *kRasterSource = R"(
    var raster = new Raster(new Size(20, 20));
    raster.position = new Point(10, 10);
    raster.setPixel(5, 5, new Color(1, 0, 0));
    var color = raster.getPixel(5, 5);
    if (Math.abs(color.red - 1) > 0.05 || Math.abs(color.green) > 0.05 || Math.abs(color.blue) > 0.05) {
      throw new Error('Pixel color mismatch: ' + color);
    }
  )";

  auto scope = qps_.CreatePaperScope();
  auto canvas = scope.Inject().CreateCanvas(100, 100);
  auto engine = scope.SetUp(canvas);
  auto exe = engine.Compile(kRasterSource);
  engine.Execute(exe);
}

TEST_F(E2eTest, RasterFromDataUrl) {
  const char *kRasterDataUrlSource = R"(
    var c = createCanvas(4, 4);
    var ctx = c.getContext('2d');
    ctx.fillStyle = '#00ff00';
    ctx.fillRect(0, 0, 4, 4);
    var dataUrl = c.toDataURL('image/png');

    var raster = new Raster(dataUrl);
    raster.position = new Point(20, 20);
    var color = raster.getPixel(2, 2);
    if (Math.abs(color.green - 1) > 0.05) {
      throw new Error('Raster data URL pixel mismatch: ' + color);
    }
  )";

  auto scope = qps_.CreatePaperScope();
  auto canvas = scope.Inject().CreateCanvas(100, 100);
  auto engine = scope.SetUp(canvas);
  auto exe = engine.Compile(kRasterDataUrlSource);
  engine.Execute(exe);
}

static std::filesystem::path FindSmileyPngPath() {
  if (const char *src_root = std::getenv("MESON_SOURCE_ROOT")) {
    auto p = std::filesystem::path(src_root) / "examples" / "smiley.png";
    if (std::filesystem::exists(p)) {
      return std::filesystem::canonical(p);
    }
  }
  const std::vector<std::filesystem::path> candidates = {
      "../../examples/smiley.png",
      "examples/smiley.png",
      "../examples/smiley.png",
  };
  for (const auto &p : candidates) {
    if (std::filesystem::exists(p)) {
      return std::filesystem::canonical(p);
    }
  }
  return "examples/smiley.png";
}

TEST_F(E2eTest, RasterFromSmileyPngFile) {
  auto smiley_path = FindSmileyPngPath();
  ASSERT_TRUE(std::filesystem::exists(smiley_path));

  // 1. Direct filesystem paths in PaperScript are disallowed for security.
  std::string disallowed_source =
      "var unauth = new Raster('" + smiley_path.string() +
      "');\n"
      "if (unauth.size.width !== 0 || unauth.size.height !== 0) {\n"
      "  throw new Error('Direct filesystem read should be disallowed');\n"
      "}\n";

  // 2. Pre-registered images via host API resolve properly.
  qps_.RegisterImage("smiley", smiley_path);

  std::string source =
      disallowed_source +
      "var raster = new Raster('smiley');\n"
      "raster.position = new Point(100, 100);\n"
      "var size = raster.size;\n"
      "if (size.width !== 200 || size.height !== 200) {\n"
      "  throw new Error('Unexpected size: ' + size);\n"
      "}\n"
      "var center = raster.getPixel(100, 100);\n"
      "if (center.red < 0.8 || center.green < 0.8 || center.blue > 0.3) {\n"
      "  throw new Error('Center color mismatch: ' + center);\n"
      "}\n"
      "var eye = raster.getPixel(70, 65);\n"
      "if (eye.red > 0.1 || eye.green > 0.1 || eye.blue > 0.1) {\n"
      "  throw new Error('Eye color mismatch: ' + eye);\n"
      "}\n";

  auto scope = qps_.CreatePaperScope();
  auto canvas = scope.Inject().CreateCanvas(200, 200);
  auto engine = scope.SetUp(canvas);
  auto exe = engine.Compile(source);
  engine.Execute(exe);
}

}  // namespace
