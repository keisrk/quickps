#ifndef QUICKPS_CANVAS_CONTEXT_H_
#define QUICKPS_CANVAS_CONTEXT_H_

#include <cairomm/cairomm.h>
#include <cairomm/surface.h>
#include <exception>
#include <filesystem>
#include <libquickps/canvas/style.hpp>
#include <libquickps/quickjsmm/esm.hpp>
#include <memory>
#include <optional>
#include <string> // Prefer enum ...
#include <type_traits>
#include <variant>

namespace quickps {
namespace canvas {

class Unimplemented : public std::exception {};

class Context final {
public:
  Context(Cairo::RefPtr<Cairo::Context> ctx);
  Cairo::RefPtr<Cairo::Context> &Impl();
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
  void Fill();
  void Fill(const std::string &fill_rule);
  void Stroke();
  void Clip();
  void Clip(const std::string &fill_rule);
  bool IsPointInPath(double x, double y);
  bool IsPointInPath(double x, double y, const std::string &fill_rule);
  void FillText(double text, double x, double y, double maxWidth);
  void StrokeText(double text, double x, double y, double maxWidth);
  void MeasureText(double text);
  void DrawImage(double image, double dx, double dy);
  void DrawImage(double image, double dx, double dy, double dw, double dh);
  void DrawImage(double image, double sx, double sy, double sw, double sh,
                 double dx, double dy, double dw, double dh);
  void CreateImageData(double sw, double sh);
  void CreateImageData(double imagedata);
  void GetImageData(double sx, double sy, double sw, double sh);
  void PutImageData(double imagedata, double dx, double dy, double dirtyX,
                    double dirtyY, double dirtyWidth, double dirtyHeight);
  void SetLineDash(double array);
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
  int font;
  int textAlign;
  int textBaseline;
  int lineDashOffset;

private:
  Cairo::RefPtr<Cairo::Context> context_;
  std::optional<std::add_pointer<Cairo::Path>::type> path_;
  void SavePath();
  void RestorePath();
};

class Canvas final {
public:
  static quickjs::Entry &CreateCanvas();
  Canvas(int width, int height);
  Canvas(const Canvas &) = delete;
  Canvas &operator=(const Canvas &) = delete;
  Cairo::RefPtr<Cairo::ImageSurface> &Impl();
  const Context CreateContext();
  const char *GetAttribute(const char *attr);
  bool HasAttribute(const char *attr);
  void Write(std::filesystem::path &png);

private:
  Cairo::RefPtr<Cairo::ImageSurface> surface_;
};

} // namespace canvas

namespace quickjs {

template <> const Class &GetClass<canvas::Context>();

template <>
std::unique_ptr<canvas::Context, OpaqueDeleter<canvas::Context>>
New<canvas::Context>(Context &ctx, ValueIter first, ValueIter last);

template <> const Class &GetClass<canvas::Canvas>();

template <>
std::unique_ptr<canvas::Canvas, OpaqueDeleter<canvas::Canvas>>
New<canvas::Canvas>(Context &ctx, ValueIter first, ValueIter last);

} // namespace quickjs
} // namespace quickps

#endif // QUICKPS_CANVAS_CONTEXT_H_
