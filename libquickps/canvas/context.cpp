#include <cairomm/context.h>
#include <cairomm/matrix.h>
#include <libquickps/canvas/facade.hpp>

namespace quickps {
namespace canvas {

Context::Context(Cairo::RefPtr<Cairo::Context> ctx) : context_(ctx) {}

Cairo::RefPtr<Cairo::Context> &Context::Impl() { return context_; }

void Context::SavePath() {
  path_ = Impl()->copy_path_flat();
  Impl()->begin_new_path();
}

void Context::RestorePath() {
  Impl()->begin_new_path();
  Impl()->append_path(*path_.value());
  path_.reset();
}

void Context::Save() { Impl()->save(); }
void Context::Restore() { Impl()->restore(); }
void Context::Scale(double x, double y) { Impl()->scale(x, y); }
void Context::Rotate(double angle) { Impl()->rotate(angle); }
void Context::Translate(double x, double y) { Impl()->translate(x, y); }
void Context::Transform(double a, double b, double c, double d, double e,
                        double f) {
  const Cairo::Matrix m(a, b, c, d, e, f);
  Impl()->transform(m);
}

void Context::SetTransform(double a, double b, double c, double d, double e,
                           double f) {
  const Cairo::Matrix m(a, b, c, d, e, f);
  Impl()->set_matrix(m);
}

Style Context::CreateLinearGradient([[maybe_unused]] double x0,
                                    [[maybe_unused]] double y0,
                                    [[maybe_unused]] double x1,
                                    [[maybe_unused]] double y1) {
  throw Unimplemented();
}

Style Context::CreateRadialGradient([[maybe_unused]] double x0,
                                    [[maybe_unused]] double y0,
                                    [[maybe_unused]] double r0,
                                    [[maybe_unused]] double x1,
                                    [[maybe_unused]] double y1,
                                    [[maybe_unused]] double r1) {
  throw Unimplemented();
}

Style Context::CreatePattern([[maybe_unused]] double image,
                             [[maybe_unused]] int repetition) {
  throw Unimplemented();
}

void Context::ClearRect(double x, double y, double width, double height) {
  Impl()->save();
  SavePath();
  Impl()->rectangle(x, y, width, height);
  Impl()->set_operator(Cairo::OPERATOR_CLEAR);
  Impl()->fill();
  RestorePath();
  Impl()->restore();
}

void Context::FillRect(double x, double y, double width, double height) {
  if (0 == width || 0 == height)
    return;

  SavePath();
  Impl()->rectangle(x, y, width, height);
  /*TODO: Needs custom logic here. */ Impl()->fill();
  RestorePath();
}

void Context::StrokeRect(double x, double y, double width, double height) {
  if (0 == width || 0 == height)
    return;

  SavePath();
  Impl()->rectangle(x, y, width, height);
  /*TODO: Needs custom logic here. */ Impl()->stroke();
  RestorePath();
}

void Context::BeginPath() { Impl()->begin_new_path(); }

void Context::ClosePath() { Impl()->close_path(); }

void Context::MoveTo(double x, double y) { Impl()->move_to(x, y); }

void Context::LineTo(double x, double y) { Impl()->line_to(x, y); }

void Context::QuadraticCurveTo(double cpx, double cpy, double x, double y) {
  double current_x, current_y;
  Impl()->get_current_point(current_x, current_y);

  if (0 == current_x && 0 == current_y) {
    current_x = cpx;
    current_y = cpy;
  }

  Impl()->curve_to(current_x + 2.0 / 3.0 * (cpx - current_x),
                   current_y + 2.0 / 3.0 * (cpy - current_y),
                   x + 2.0 / 3.0 * (cpx - x), y + 2.0 / 3.0 * (cpy - y), x, y);
}

void Context::BezierCurveTo(double cp1x, double cp1y, double cp2x, double cp2y,
                            double x, double y) {
  Impl()->curve_to(cp1x, cp1y, cp2x, cp2y, x, y);
}

void Context::ArcTo([[maybe_unused]] double x1, [[maybe_unused]] double y1,
                    [[maybe_unused]] double x2, [[maybe_unused]] double y2,
                    [[maybe_unused]] double radius) {
  throw Unimplemented();
}

void Context::Rect(double x, double y, double width, double height) {
  Impl()->rectangle(x, y, width, height);
}

void Context::Arc([[maybe_unused]] double x, [[maybe_unused]] double y,
                  [[maybe_unused]] double radius,
                  [[maybe_unused]] double startAngle,
                  [[maybe_unused]] double endAngle,
                  [[maybe_unused]] double anticlockwise) {
  throw Unimplemented();
}

void Context::Fill() { Impl()->fill(); }

void Context::Fill(const std::string &fill_rule) {
  auto rule = "evenodd" == fill_rule ? Cairo::FILL_RULE_EVEN_ODD
                                     : Cairo::FILL_RULE_WINDING;
  Impl()->set_fill_rule(rule);
  Impl()->fill();
}

void Context::Stroke() { Impl()->stroke(); }

void Context::Clip() { Impl()->clip(); }

void Context::Clip(const std::string &fill_rule) {
  auto rule = "evenodd" == fill_rule ? Cairo::FILL_RULE_EVEN_ODD
                                     : Cairo::FILL_RULE_WINDING;
  Impl()->set_fill_rule(rule);
  Impl()->clip();
}

bool Context::IsPointInPath(double x, double y) {
  return Impl()->in_fill(x, y) || Impl()->in_stroke(x, y);
}

bool Context::IsPointInPath(double x, double y, const std::string &fill_rule) {
  auto rule = "evenodd" == fill_rule ? Cairo::FILL_RULE_EVEN_ODD
                                     : Cairo::FILL_RULE_WINDING;
  Impl()->set_fill_rule(rule);
  auto result = IsPointInPath(x, y);
  return result;
}

void Context::FillText([[maybe_unused]] double text, [[maybe_unused]] double x,
                       [[maybe_unused]] double y,
                       [[maybe_unused]] double maxWidth) {}
void Context::StrokeText([[maybe_unused]] double text,
                         [[maybe_unused]] double x, [[maybe_unused]] double y,
                         [[maybe_unused]] double maxWidth) {}
void Context::MeasureText([[maybe_unused]] double text) {}
void Context::DrawImage([[maybe_unused]] double image,
                        [[maybe_unused]] double dx,
                        [[maybe_unused]] double dy) {}
void Context::DrawImage([[maybe_unused]] double image,
                        [[maybe_unused]] double dx, [[maybe_unused]] double dy,
                        [[maybe_unused]] double dw,
                        [[maybe_unused]] double dh) {}
void Context::DrawImage([[maybe_unused]] double image,
                        [[maybe_unused]] double sx, [[maybe_unused]] double sy,
                        [[maybe_unused]] double sw, [[maybe_unused]] double sh,
                        [[maybe_unused]] double dx, [[maybe_unused]] double dy,
                        [[maybe_unused]] double dw,
                        [[maybe_unused]] double dh) {}
void Context::CreateImageData([[maybe_unused]] double sw,
                              [[maybe_unused]] double sh) {
  throw Unimplemented();
}
void Context::CreateImageData([[maybe_unused]] double imagedata) {
  throw Unimplemented();
}
void Context::GetImageData([[maybe_unused]] double sx,
                           [[maybe_unused]] double sy,
                           [[maybe_unused]] double sw,
                           [[maybe_unused]] double sh) {}
void Context::PutImageData([[maybe_unused]] double imagedata,
                           [[maybe_unused]] double dx,
                           [[maybe_unused]] double dy,
                           [[maybe_unused]] double dirtyX,
                           [[maybe_unused]] double dirtyY,
                           [[maybe_unused]] double dirtyWidth,
                           [[maybe_unused]] double dirtyHeight) {}
void Context::SetLineDash([[maybe_unused]] double array) {}

Canvas::Canvas(int width, int height)
    : surface_(Cairo::ImageSurface::create(Cairo::Format::FORMAT_ARGB32, width,
                                           height)) {}

Cairo::RefPtr<Cairo::ImageSurface> &Canvas::Impl() { return surface_; }

const Context Canvas::CreateContext() {
  return Context(Cairo::Context::create(surface_));
}

const char *Canvas::GetAttribute([[maybe_unused]] const char *attr) {
  return "nope";
}

bool Canvas::HasAttribute([[maybe_unused]] const char *attr) { return false; }

void Canvas::Write(std::filesystem::path &png) { surface_->write_to_png(png); }

quickjs::Entry &Canvas::CreateCanvas() {
  static quickjs::Entry kEntry = quickjs::EntryBuilder<>()
                                     .Name("createContext")
                                     .Func<facade::CreateCanvas>(3)
                                     .Build();
  return kEntry;
}

} // namespace canvas

namespace quickjs {

using namespace canvas::facade;

template <> const Class &GetClass<canvas::Context>() {
  static Class kClass =
      ClassBuilder<canvas::Context>("CanvasRenderingContext2D")
          .Constructor<New<canvas::Context>>(1)
          .Proto(EntryBuilder<canvas::Context>()
                     .Name("fillStyle")
                     .Getter<GetFillStyle>()
                     .Setter<SetFillStyle>()
                     .Build())
          .Proto(EntryBuilder<canvas::Context>()
                     .Name("strokeStyle")
                     .Getter<GetStrokeStyle>()
                     .Setter<SetStrokeStyle>()
                     .Build())
          .Proto(EntryBuilder<canvas::Context>()
                     .Name("save")
                     .Method<Save>(0)
                     .Build())
          .Proto(EntryBuilder<canvas::Context>()
                     .Name("restore")
                     .Method<Restore>(0)
                     .Build())
          .Proto(EntryBuilder<canvas::Context>()
                     .Name("scale")
                     .Method<Scale>(2)
                     .Build())
          .Proto(EntryBuilder<canvas::Context>()
                     .Name("rotate")
                     .Method<Rotate>(1)
                     .Build())
          .Proto(EntryBuilder<canvas::Context>()
                     .Name("translate")
                     .Method<Translate>(2)
                     .Build())
          .Proto(EntryBuilder<canvas::Context>()
                     .Name("transform")
                     .Method<Transform>(6)
                     .Build())
          .Proto(EntryBuilder<canvas::Context>()
                     .Name("setTransform")
                     .Method<SetTransform>(6)
                     .Build())
          .Proto(EntryBuilder<canvas::Context>()
                     .Name("createLinearGradient")
                     .Method<CreateLinearGradient>(4)
                     .Build())
          .Proto(EntryBuilder<canvas::Context>()
                     .Name("createRadialGradient")
                     .Method<CreateRadialGradient>(6)
                     .Build())
          .Proto(EntryBuilder<canvas::Context>()
                     .Name("createPattern")
                     .Method<CreatePattern>(2)
                     .Build())
          .Proto(EntryBuilder<canvas::Context>()
                     .Name("clearRect")
                     .Method<ClearRect>(4)
                     .Build())
          .Proto(EntryBuilder<canvas::Context>()
                     .Name("fillRect")
                     .Method<FillRect>(4)
                     .Build())
          .Proto(EntryBuilder<canvas::Context>()
                     .Name("strokeRect")
                     .Method<StrokeRect>(4)
                     .Build())
          .Proto(EntryBuilder<canvas::Context>()
                     .Name("beginPath")
                     .Method<BeginPath>(0)
                     .Build())
          .Proto(EntryBuilder<canvas::Context>()
                     .Name("closePath")
                     .Method<ClosePath>(0)
                     .Build())
          .Proto(EntryBuilder<canvas::Context>()
                     .Name("moveTo")
                     .Method<MoveTo>(2)
                     .Build())
          .Proto(EntryBuilder<canvas::Context>()
                     .Name("lineTo")
                     .Method<LineTo>(2)
                     .Build())
          .Proto(EntryBuilder<canvas::Context>()
                     .Name("quadraticCurveTo")
                     .Method<QuadraticCurveTo>(4)
                     .Build())
          .Proto(EntryBuilder<canvas::Context>()
                     .Name("bezierCurveTo")
                     .Method<BezierCurveTo>(6)
                     .Build())
          .Proto(EntryBuilder<canvas::Context>()
                     .Name("arcTo")
                     .Method<ArcTo>(5)
                     .Build())
          .Proto(EntryBuilder<canvas::Context>()
                     .Name("rect")
                     .Method<Rect>(4)
                     .Build())
          .Proto(EntryBuilder<canvas::Context>()
                     .Name("arc")
                     .Method<Arc>(6)
                     .Build())
          .Proto(EntryBuilder<canvas::Context>()
                     .Name("fill")
                     .Method<Fill>(0)
                     .Build())
          .Proto(EntryBuilder<canvas::Context>()
                     .Name("stroke")
                     .Method<Stroke>(0)
                     .Build())
          .Proto(EntryBuilder<canvas::Context>()
                     .Name("clip")
                     .Method<Clip>(1)
                     .Build())
          .Proto(EntryBuilder<canvas::Context>()
                     .Name("isPointInPath")
                     .Method<IsPointInPath>(3)
                     .Build())
          .Build();

  return kClass;
}

template <>
std::unique_ptr<canvas::Context, OpaqueDeleter<canvas::Context>>
New<canvas::Context>(Context &ctx, ValueIter first, ValueIter last) {
  auto canvas = ctx.GetOpaque<canvas::Canvas>(*first++);

  if (first != last) {
    throw Exception();
  }

  return Runtime::GetInstance().Ctor<canvas::Context>(canvas->CreateContext());
}

template <> const Class &GetClass<canvas::Canvas>() {
  static Class kClass = ClassBuilder<canvas::Canvas>("HTMLCanvasElement")
                            .Constructor<New<canvas::Canvas>>(3)
                            .Proto(EntryBuilder<canvas::Canvas>()
                                       .Name("getContext")
                                       .Method<GetContext>(1)
                                       .Build())
                            .Proto(EntryBuilder<canvas::Canvas>()
                                       .Name("width")
                                       .Getter<get_width>()
                                       .Setter<set_width>()
                                       .Build())
                            .Proto(EntryBuilder<canvas::Canvas>()
                                       .Name("height")
                                       .Getter<get_height>()
                                       .Setter<set_height>()
                                       .Build())
                            .Proto(EntryBuilder<canvas::Canvas>()
                                       .Name("type")
                                       .Getter<get_type>()
                                       .Setter<set_type>()
                                       .Build())
                            .Proto(EntryBuilder<canvas::Canvas>()
                                       .Name("getAttribute")
                                       .Method<GetAttribute>(1)
                                       .Build())
                            .Proto(EntryBuilder<canvas::Canvas>()
                                       .Name("hasAttribute")
                                       .Method<HasAttribute>(1)
                                       .Build())
                            .Proto(EntryBuilder<canvas::Canvas>()
                                       .Name("addEventListener")
                                       .Method<AddEventListener>(3)
                                       .Build())
                            .Proto(EntryBuilder<canvas::Canvas>()
                                       .Name("style")
                                       .Getter<GetStyle>()
                                       .Setter<SetStyle>()
                                       .Build())
                            .Proto(EntryBuilder<canvas::Canvas>()
                                       .Name("ownerDocument")
                                       .Getter<GetOwnerDocument>()
                                       .Setter<SetOwnerDocument>()
                                       .Build())
                            .Build();

  return kClass;
}

template <>
std::unique_ptr<canvas::Canvas, OpaqueDeleter<canvas::Canvas>>
New<canvas::Canvas>(Context &ctx, ValueIter first, ValueIter last) {
  auto width = first++->Get<int>(ctx);
  auto height = first++->Get<int>(ctx);
  [[maybe_unused]] auto type = first++->Get<std::string>(ctx);

  if (first != last) {
    throw Exception();
  }

  return Runtime::GetInstance().Ctor<canvas::Canvas>(width, height);
}

} // namespace quickjs
} // namespace quickps
