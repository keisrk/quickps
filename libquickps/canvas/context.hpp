#ifndef QUICKPS_CANVAS_CONTEXT_H_
#define QUICKPS_CANVAS_CONTEXT_H_

#include <blend2d/blend2d.h>

#include <exception>
#include <filesystem>
#include <libquickps/canvas/font.hpp>
#include <libquickps/canvas/style.hpp>
#include <libquickps/quickjsmm/esm.hpp>
#include <memory>
#include <optional>
#include <string>  // Prefer enum ...
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

namespace quickps {
namespace canvas {

class Unimplemented : public std::exception {};
class Canvas;

struct ImageDataBuffer {
  int width = 0;
  int height = 0;
  std::vector<uint8_t> data;
};

struct TextMetricsResult {
  double width = 0.0;
  double actual_bounding_box_left = 0.0;
  double actual_bounding_box_right = 0.0;
  double actual_bounding_box_ascent = 0.0;
  double actual_bounding_box_descent = 0.0;
  double font_bounding_box_ascent = 0.0;
  double font_bounding_box_descent = 0.0;
  double em_height_ascent = 0.0;
  double em_height_descent = 0.0;
};

struct TextState {
  std::string font_str = "10px sans-serif";
  ParsedFontSpec font_spec{
      .style = BL_FONT_STYLE_NORMAL,
      .weight = BL_FONT_WEIGHT_NORMAL,
      .stretch = BL_FONT_STRETCH_NORMAL,
      .small_caps = false,
      .size_px = 10.0f,
      .families = {"sans-serif"},
  };
  BLFont font;
  std::string text_align = "start";
  std::string text_baseline = "alphabetic";
  std::string direction = "ltr";
  std::string letter_spacing_str = "0px";
  double letter_spacing_px = 0.0;
};

class Context final {
 public:
  explicit Context(Canvas *canvas = nullptr);
  explicit Context(BLContext ctx);
  ~Context();

  Canvas *canvas() const { return canvas_; }
  void DetachCanvas() { canvas_ = nullptr; }
  void Rebind();

  BLContext &Impl();
  void Save();
  void Restore();
  void Scale(double x, double y);
  void Rotate(double angle);
  void Translate(double x, double y);
  void Transform(double a, double b, double c, double d, double e, double f);
  void SetTransform(double a, double b, double c, double d, double e, double f);
  Style CreateLinearGradient(double x0, double y0, double x1, double y1);
  Style CreateRadialGradient(double x0, double y0, double r0, double x1,
                             double y1, double r1);
  Style CreatePattern(double image, int repetition);
  Style CreatePattern(Canvas &image, const std::string &repetition);
  void ClearRect(double x, double y, double w, double h);
  void FillRect(double x, double y, double w, double h);
  void StrokeRect(double x, double y, double w, double h);
  void BeginPath();
  void ClosePath();
  void MoveTo(double x, double y);
  void LineTo(double x, double y);
  void QuadraticCurveTo(double cpx, double cpy, double x, double y);
  void BezierCurveTo(double cp1x, double cp1y, double cp2x, double cp2y,
                     double x, double y);
  void ArcTo(double x1, double y1, double x2, double y2, double radius);
  void Rect(double x, double y, double w, double h);
  void Arc(double x, double y, double radius, double startAngle,
           double endAngle, double anticlockwise);
  void Arc(double x, double y, double radius, double startAngle,
           double endAngle, bool anticlockwise = false);
  void Fill();
  void Fill(const std::string &fill_rule);
  void Stroke();
  void Clip();
  void Clip(const std::string &fill_rule);
  bool IsPointInPath(double x, double y);
  bool IsPointInPath(double x, double y, const std::string &fill_rule);
  void FillText(const std::string &text, double x, double y,
                std::optional<double> max_width = std::nullopt);
  void StrokeText(const std::string &text, double x, double y,
                  std::optional<double> max_width = std::nullopt);
  TextMetricsResult MeasureText(const std::string &text);
  void DrawImage(const BLImage &image, double dx, double dy);
  void DrawImage(const BLImage &image, const BLRect &dst);
  void DrawImage(const BLImage &image, const BLRect &dst, const BLRectI &src);
  ImageDataBuffer GetImageData(int sx, int sy, int sw, int sh);
  void PutImageData(const ImageDataBuffer &data, int dx, int dy,
                    const BLRectI &dirty_rect);
  bool GetImageSmoothingEnabled() const;
  void SetImageSmoothingEnabled(bool enabled);
  void SetLineDash(double array);
  void SetLineDash(const std::vector<double> &segments);
  std::vector<double> GetLineDash() const;
  double GetLineDashOffset() const;
  void SetLineDashOffset(double offset);
  double GetLineWidth() const;
  void SetLineWidth(double width);
  std::string GetLineCap() const;
  void SetLineCap(const std::string &cap);
  std::string GetLineJoin() const;
  void SetLineJoin(const std::string &join);
  double GetMiterLimit() const;
  void SetMiterLimit(double limit);
  double GetGlobalAlpha() const;
  void SetGlobalAlpha(double alpha);
  std::string GetGlobalCompositeOperation() const;
  void SetGlobalCompositeOperation(const std::string &op);
  std::string GetFont() const;
  void SetFont(const std::string &font);
  std::string GetTextAlign() const;
  void SetTextAlign(const std::string &align);
  std::string GetTextBaseline() const;
  void SetTextBaseline(const std::string &baseline);
  std::string GetDirection() const;
  void SetDirection(const std::string &dir);
  std::string GetLetterSpacing() const;
  void SetLetterSpacing(const std::string &spacing);
  Style GetFillStyle();
  void SetFillStyle(const Style &);
  void SetFillStyle(const Color &);
  Style GetStrokeStyle();
  void SetStrokeStyle(const Style &);
  void SetStrokeStyle(const Color &);

  double globalAlpha = 0.0;
  double globalCompositeOperation = 0.0;
  double lineWidth = 1.0;
  std::string lineCap = "";
  std::string lineJoin = "";
  double miterLimit = 0.0;
  double shadowOffsetX = 0.0;
  double shadowOffsetY = 0.0;
  double shadowBlur = 0.0;
  std::array<int, 4> shadowColor = {0, 0, 0, 0};
  int lineDashOffset;

 private:
  const BLFont &EnsureFontResolved();

  Canvas *canvas_ = nullptr;
  BLContext context_;
  BLPath path_;
  std::optional<BLImage> clip_mask_;
  std::vector<std::optional<BLImage>> clip_stack_;
  TextState text_state_;
  std::vector<TextState> text_state_stack_;
  std::unordered_map<std::string, std::pair<ParsedFontSpec, BLFont>>
      font_cache_;
  bool image_smoothing_enabled_ = true;
};

class Canvas final {
 public:
  static quickjs::Entry &CreateCanvas();
  Canvas(int width, int height);
  ~Canvas();
  Canvas(const Canvas &) = delete;
  Canvas &operator=(const Canvas &) = delete;
  BLImage &Impl();
  const BLImage &Impl() const;
  int width() const { return width_; }
  int height() const { return height_; }
  void SetWidth(int width);
  void SetHeight(int height);
  void RegisterContext(Context *ctx);
  void UnregisterContext(Context *ctx);
  const Context CreateContext();
  const char *GetAttribute(const char *attr);
  bool HasAttribute(const char *attr);
  void Write(std::filesystem::path &png);
  std::string ToDataURL(const std::string &type = "image/png");

 private:
  int width_ = 0;
  int height_ = 0;
  BLImage surface_;
  Context *active_context_ = nullptr;
};

void RegisterCanvasModule(quickjs::Context &ctx);

}  // namespace canvas

namespace quickjs {

template <>
const Class &GetClass<canvas::Context>();

template <>
std::unique_ptr<canvas::Context, OpaqueDeleter<canvas::Context>>
New<canvas::Context>(Context &ctx, ValueIter first, ValueIter last);

template <>
const Class &GetClass<canvas::Canvas>();

template <>
std::unique_ptr<canvas::Canvas, OpaqueDeleter<canvas::Canvas>>
New<canvas::Canvas>(Context &ctx, ValueIter first, ValueIter last);

}  // namespace quickjs
}  // namespace quickps

#endif  // QUICKPS_CANVAS_CONTEXT_H_
