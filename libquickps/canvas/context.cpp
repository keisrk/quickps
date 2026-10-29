#include <libquickps/canvas/facade.hpp>
#include <utility>

namespace quickps {
namespace canvas {

namespace {

BLFillRule ParseFillRule(const std::string &fill_rule) {
  return "evenodd" == fill_rule ? BL_FILL_RULE_EVEN_ODD : BL_FILL_RULE_NON_ZERO;
}

template <typename DrawFn>
void DrawWithClipMask(BLContext &ctx, const std::optional<BLImage> &clip_mask,
                      DrawFn &&draw_fn) {
  if (!clip_mask.has_value()) {
    draw_fn(ctx);
    return;
  }

  const int w = static_cast<int>(ctx.target_width());
  const int h = static_cast<int>(ctx.target_height());
  BLImage layer(w, h, BL_FORMAT_PRGB32);
  BLContext layer_ctx(layer);
  layer_ctx.clear_all();
  layer_ctx.set_hints(ctx.hints());
  layer_ctx.set_approximation_options(ctx.approximation_options());
  layer_ctx.set_global_alpha(ctx.global_alpha());
  layer_ctx.set_fill_rule(ctx.fill_rule());
  layer_ctx.set_stroke_options(ctx.stroke_options());

  BLVar fill_style;
  ctx.get_fill_style(fill_style);
  layer_ctx.set_fill_style(fill_style);
  layer_ctx.set_fill_alpha(ctx.fill_alpha());

  BLVar stroke_style;
  ctx.get_stroke_style(stroke_style);
  layer_ctx.set_stroke_style(stroke_style);
  layer_ctx.set_stroke_alpha(ctx.stroke_alpha());

  layer_ctx.set_transform(ctx.user_transform());
  draw_fn(layer_ctx);

  layer_ctx.reset_transform();
  layer_ctx.set_comp_op(BL_COMP_OP_DST_IN);
  layer_ctx.blit_image(BLPointI(0, 0), *clip_mask);
  layer_ctx.end();

  ctx.save();
  ctx.reset_transform();
  ctx.blit_image(BLPointI(0, 0), layer);
  ctx.restore();
}

} // namespace

Context::Context(BLContext ctx) : context_(std::move(ctx)) {}

BLContext &Context::Impl() { return context_; }

void Context::Save() {
  clip_stack_.push_back(clip_mask_);
  Impl().save();
}

void Context::Restore() {
  if (!clip_stack_.empty()) {
    clip_mask_ = std::move(clip_stack_.back());
    clip_stack_.pop_back();
  }
  Impl().restore();
}

void Context::Scale(double x, double y) { Impl().scale(x, y); }
void Context::Rotate(double angle) { Impl().rotate(angle); }
void Context::Translate(double x, double y) { Impl().translate(x, y); }
void Context::Transform(double a, double b, double c, double d, double e,
                        double f) {
  const BLMatrix2D m(a, b, c, d, e, f);
  Impl().apply_transform(m);
}

void Context::SetTransform(double a, double b, double c, double d, double e,
                           double f) {
  const BLMatrix2D m(a, b, c, d, e, f);
  Impl().set_transform(m);
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
  if (!clip_mask_.has_value()) {
    Impl().clear_rect(x, y, width, height);
    return;
  }

  const int w = static_cast<int>(Impl().target_width());
  const int h = static_cast<int>(Impl().target_height());
  BLImage layer(w, h, BL_FORMAT_PRGB32);
  BLContext layer_ctx(layer);
  layer_ctx.clear_all();
  layer_ctx.set_transform(Impl().user_transform());
  layer_ctx.set_fill_style(BLRgba32(0xFFFFFFFFu));
  layer_ctx.fill_rect(x, y, width, height);
  layer_ctx.reset_transform();
  layer_ctx.set_comp_op(BL_COMP_OP_DST_IN);
  layer_ctx.blit_image(BLPointI(0, 0), *clip_mask_);
  layer_ctx.end();

  Impl().save();
  Impl().reset_transform();
  Impl().set_comp_op(BL_COMP_OP_DST_OUT);
  Impl().blit_image(BLPointI(0, 0), layer);
  Impl().restore();
}

void Context::FillRect(double x, double y, double width, double height) {
  if (0 == width || 0 == height)
    return;

  DrawWithClipMask(Impl(), clip_mask_,
                   [&](BLContext &ctx) { ctx.fill_rect(x, y, width, height); });
}

void Context::StrokeRect(double x, double y, double width, double height) {
  if (0 == width || 0 == height)
    return;

  DrawWithClipMask(Impl(), clip_mask_, [&](BLContext &ctx) {
    ctx.stroke_rect(x, y, width, height);
  });
}

void Context::BeginPath() { path_.clear(); }

void Context::ClosePath() { path_.close(); }

void Context::MoveTo(double x, double y) { path_.move_to(x, y); }

void Context::LineTo(double x, double y) { path_.line_to(x, y); }

void Context::QuadraticCurveTo(double cpx, double cpy, double x, double y) {
  path_.quad_to(cpx, cpy, x, y);
}

void Context::BezierCurveTo(double cp1x, double cp1y, double cp2x, double cp2y,
                            double x, double y) {
  path_.cubic_to(cp1x, cp1y, cp2x, cp2y, x, y);
}

void Context::ArcTo([[maybe_unused]] double x1, [[maybe_unused]] double y1,
                    [[maybe_unused]] double x2, [[maybe_unused]] double y2,
                    [[maybe_unused]] double radius) {
  throw Unimplemented();
}

void Context::Rect(double x, double y, double width, double height) {
  path_.add_rect(x, y, width, height);
}

void Context::Arc([[maybe_unused]] double x, [[maybe_unused]] double y,
                  [[maybe_unused]] double radius,
                  [[maybe_unused]] double startAngle,
                  [[maybe_unused]] double endAngle,
                  [[maybe_unused]] double anticlockwise) {
  throw Unimplemented();
}

void Context::Fill() {
  DrawWithClipMask(Impl(), clip_mask_,
                   [this](BLContext &ctx) { ctx.fill_path(path_); });
}

void Context::Fill(const std::string &fill_rule) {
  Impl().set_fill_rule(ParseFillRule(fill_rule));
  Fill();
}

void Context::Stroke() {
  DrawWithClipMask(Impl(), clip_mask_,
                   [this](BLContext &ctx) { ctx.stroke_path(path_); });
}

void Context::Clip() {
  const int w = static_cast<int>(Impl().target_width());
  const int h = static_cast<int>(Impl().target_height());
  BLImage new_mask(w, h, BL_FORMAT_PRGB32);
  BLContext mask_ctx(new_mask);
  mask_ctx.clear_all();
  mask_ctx.set_transform(Impl().user_transform());
  mask_ctx.set_fill_rule(Impl().fill_rule());
  mask_ctx.set_fill_style(BLRgba32(0xFFFFFFFFu));
  mask_ctx.fill_path(path_);
  if (clip_mask_.has_value()) {
    mask_ctx.reset_transform();
    mask_ctx.set_comp_op(BL_COMP_OP_DST_IN);
    mask_ctx.blit_image(BLPointI(0, 0), *clip_mask_);
  }
  mask_ctx.end();
  clip_mask_ = std::move(new_mask);
}

void Context::Clip(const std::string &fill_rule) {
  Impl().set_fill_rule(ParseFillRule(fill_rule));
  Clip();
}

bool Context::IsPointInPath(double x, double y) {
  return path_.hit_test(BLPoint(x, y), Impl().fill_rule()) == BL_HIT_TEST_IN;
}

bool Context::IsPointInPath(double x, double y, const std::string &fill_rule) {
  Impl().set_fill_rule(ParseFillRule(fill_rule));
  return IsPointInPath(x, y);
}

void Context::SetFillStyle(const Color &color) {
  Impl().set_fill_style(BLRgba(color.r, color.g, color.b, color.a));
}

void Context::SetStrokeStyle(const Color &color) {
  Impl().set_stroke_style(BLRgba(color.r, color.g, color.b, color.a));
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
    : surface_(width, height, BL_FORMAT_PRGB32) {
  BLContext(surface_).clear_all();
}

BLImage &Canvas::Impl() { return surface_; }

const Context Canvas::CreateContext() { return Context(BLContext(surface_)); }

const char *Canvas::GetAttribute([[maybe_unused]] const char *attr) {
  return "nope";
}

bool Canvas::HasAttribute([[maybe_unused]] const char *attr) { return false; }

void Canvas::Write(std::filesystem::path &png) {
  surface_.write_to_file(png.c_str());
}

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
