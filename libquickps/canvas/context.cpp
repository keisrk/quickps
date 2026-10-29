#include <algorithm>
#include <cmath>
#include <libquickps/canvas/facade.hpp>
#include <numbers>
#include <utility>

namespace quickps {
namespace canvas {

namespace {

BLFillRule ParseFillRule(const std::string &fill_rule) {
  return "evenodd" == fill_rule ? BL_FILL_RULE_EVEN_ODD : BL_FILL_RULE_NON_ZERO;
}

BLExtendMode ParseExtendMode(const std::string &repetition) {
  if (repetition == "no-repeat") {
    return BL_EXTEND_MODE_PAD;
  }
  if (repetition == "repeat-x") {
    return BL_EXTEND_MODE_REPEAT_X_PAD_Y;
  }
  if (repetition == "repeat-y") {
    return BL_EXTEND_MODE_PAD_X_REPEAT_Y;
  }
  return BL_EXTEND_MODE_REPEAT;
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
  ctx.set_global_alpha(1.0);
  ctx.blit_image(BLPointI(0, 0), layer);
  ctx.restore();
}

} // namespace

Context::Context(BLContext ctx) : context_(std::move(ctx)) {
  Impl().set_fill_style(BLRgba(0.0f, 0.0f, 0.0f, 1.0f));
  Impl().set_stroke_style(BLRgba(0.0f, 0.0f, 0.0f, 1.0f));
}

BLContext &Context::Impl() { return context_; }

void Context::Save() {
  constexpr size_t kMaxSaveDepth = 1024;
  if (clip_stack_.size() >= kMaxSaveDepth) {
    throw quickjs::Exception();
  }
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

Style Context::CreateLinearGradient(double x0, double y0, double x1,
                                    double y1) {
  return Style::FromGradient(
      BLGradient(BLLinearGradientValues(x0, y0, x1, y1)));
}

Style Context::CreateRadialGradient(double x0, double y0, double r0, double x1,
                                    double y1, double r1) {
  return Style::FromGradient(
      BLGradient(BLRadialGradientValues(x1, y1, x0, y0, r1, r0)));
}

Style Context::CreatePattern([[maybe_unused]] double image,
                             [[maybe_unused]] int repetition) {
  throw Unimplemented();
}

Style Context::CreatePattern(Canvas &image, const std::string &repetition) {
  return Style::FromPattern(
      BLPattern(image.Impl(), ParseExtendMode(repetition)));
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

void Context::ArcTo(double x1, double y1, double x2, double y2, double radius) {
  if (!std::isfinite(x1) || !std::isfinite(y1) || !std::isfinite(x2) ||
      !std::isfinite(y2) || !std::isfinite(radius)) {
    return;
  }
  if (radius < 0.0) {
    throw quickjs::Exception();
  }

  BLPoint p0;
  if (path_.get_last_vertex(&p0) != BL_SUCCESS) {
    path_.move_to(x1, y1);
    return;
  }

  const double dx0 = p0.x - x1;
  const double dy0 = p0.y - y1;
  const double len0 = std::hypot(dx0, dy0);

  const double dx1 = x2 - x1;
  const double dy1 = y2 - y1;
  const double len1 = std::hypot(dx1, dy1);

  if (len0 < 1e-9 || len1 < 1e-9 || radius == 0.0) {
    path_.line_to(x1, y1);
    return;
  }

  const double ux0 = dx0 / len0;
  const double uy0 = dy0 / len0;
  const double ux1 = dx1 / len1;
  const double uy1 = dy1 / len1;

  const double cross = ux0 * uy1 - uy0 * ux1;
  if (std::fabs(cross) < 1e-9) {
    path_.line_to(x1, y1);
    return;
  }

  const double dot = std::clamp(ux0 * ux1 + uy0 * uy1, -1.0, 1.0);
  const double half_angle = 0.5 * std::acos(dot);
  const double tan_half = std::tan(half_angle);
  if (std::fabs(tan_half) < 1e-9) {
    path_.line_to(x1, y1);
    return;
  }

  const double dist = radius / tan_half;
  const double t0x = x1 + ux0 * dist;
  const double t0y = y1 + uy0 * dist;
  const double t1x = x1 + ux1 * dist;
  const double t1y = y1 + uy1 * dist;

  // Normal from t0 toward center depends on turn direction
  const double sign = cross < 0.0 ? 1.0 : -1.0;
  const double cx = t0x + sign * (-uy0) * radius;
  const double cy = t0y + sign * ux0 * radius;

  const double start_angle = std::atan2(t0y - cy, t0x - cx);
  const double end_angle = std::atan2(t1y - cy, t1x - cx);
  double sweep = end_angle - start_angle;
  if (cross < 0.0) {
    if (sweep < 0.0) {
      sweep += 2.0 * std::numbers::pi;
    }
  } else {
    if (sweep > 0.0) {
      sweep -= 2.0 * std::numbers::pi;
    }
  }

  path_.arc_to(cx, cy, radius, radius, start_angle, sweep, false);
}

void Context::Rect(double x, double y, double width, double height) {
  path_.add_rect(x, y, width, height);
}

void Context::Arc(double x, double y, double radius, double startAngle,
                  double endAngle, double anticlockwise) {
  Arc(x, y, radius, startAngle, endAngle, anticlockwise != 0.0);
}

void Context::Arc(double x, double y, double radius, double startAngle,
                  double endAngle, bool anticlockwise) {
  if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(radius) ||
      !std::isfinite(startAngle) || !std::isfinite(endAngle)) {
    return;
  }
  if (radius < 0.0) {
    throw quickjs::Exception();
  }

  constexpr double kTwoPi = 2.0 * std::numbers::pi;
  double sweep = endAngle - startAngle;
  if (!anticlockwise) {
    if (sweep >= kTwoPi) {
      sweep = kTwoPi;
    } else {
      while (sweep < 0.0) {
        sweep += kTwoPi;
      }
    }
  } else {
    if (sweep <= -kTwoPi) {
      sweep = -kTwoPi;
    } else {
      while (sweep > 0.0) {
        sweep -= kTwoPi;
      }
    }
  }

  path_.arc_to(x, y, radius, radius, startAngle, sweep, false);
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

Style Context::GetFillStyle() {
  BLVar var;
  Impl().get_fill_style(var);
  if (var.type() == BL_OBJECT_TYPE_GRADIENT) {
    return Style::FromGradient(var.as<BLGradient>());
  }
  if (var.type() == BL_OBJECT_TYPE_PATTERN) {
    return Style::FromPattern(var.as<BLPattern>());
  }
  BLRgba rgba{};
  if (bl_var_to_rgba(&var, &rgba) == BL_SUCCESS) {
    return Style::FromColor(Color(rgba.r, rgba.g, rgba.b, rgba.a));
  }
  return Style::FromColor(Color(0.0f, 0.0f, 0.0f, 1.0f));
}

void Context::SetFillStyle(const Style &style) {
  std::visit(
      [this](const auto &s) {
        using T = std::decay_t<decltype(s)>;
        if constexpr (std::is_same_v<T, Color>) {
          SetFillStyle(s);
        } else {
          Impl().set_fill_style(s);
        }
      },
      style.value);
}

void Context::SetFillStyle(const Color &color) {
  Impl().set_fill_style(BLRgba(color.r, color.g, color.b, color.a));
}

Style Context::GetStrokeStyle() {
  BLVar var;
  Impl().get_stroke_style(var);
  if (var.type() == BL_OBJECT_TYPE_GRADIENT) {
    return Style::FromGradient(var.as<BLGradient>());
  }
  if (var.type() == BL_OBJECT_TYPE_PATTERN) {
    return Style::FromPattern(var.as<BLPattern>());
  }
  BLRgba rgba{};
  if (bl_var_to_rgba(&var, &rgba) == BL_SUCCESS) {
    return Style::FromColor(Color(rgba.r, rgba.g, rgba.b, rgba.a));
  }
  return Style::FromColor(Color(0.0f, 0.0f, 0.0f, 1.0f));
}

void Context::SetStrokeStyle(const Style &style) {
  std::visit(
      [this](const auto &s) {
        using T = std::decay_t<decltype(s)>;
        if constexpr (std::is_same_v<T, Color>) {
          SetStrokeStyle(s);
        } else {
          Impl().set_stroke_style(s);
        }
      },
      style.value);
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

void Context::SetLineDash(const std::vector<double> &segments) {
  for (double v : segments) {
    if (!std::isfinite(v) || v < 0.0) {
      return;
    }
  }
  BLArray<double> dash;
  if (segments.size() % 2 == 1) {
    dash.reserve(segments.size() * 2);
    dash.append_data(segments.data(), segments.size());
    dash.append_data(segments.data(), segments.size());
  } else {
    dash.assign_data(segments.data(), segments.size());
  }
  Impl().set_stroke_dash_array(dash);
}

std::vector<double> Context::GetLineDash() const {
  const auto &dash = context_.stroke_dash_array();
  return std::vector<double>(dash.data(), dash.data() + dash.size());
}

double Context::GetLineDashOffset() const {
  return context_.stroke_dash_offset();
}

void Context::SetLineDashOffset(double offset) {
  if (std::isfinite(offset)) {
    Impl().set_stroke_dash_offset(offset);
  }
}

double Context::GetLineWidth() const { return context_.stroke_width(); }

void Context::SetLineWidth(double width) {
  if (std::isfinite(width) && width > 0.0) {
    lineWidth = width;
    Impl().set_stroke_width(width);
  }
}

std::string Context::GetLineCap() const {
  switch (context_.stroke_start_cap()) {
  case BL_STROKE_CAP_ROUND:
    return "round";
  case BL_STROKE_CAP_SQUARE:
    return "square";
  default:
    return "butt";
  }
}

void Context::SetLineCap(const std::string &cap) {
  if (cap == "butt") {
    lineCap = cap;
    Impl().set_stroke_caps(BL_STROKE_CAP_BUTT);
  } else if (cap == "round") {
    lineCap = cap;
    Impl().set_stroke_caps(BL_STROKE_CAP_ROUND);
  } else if (cap == "square") {
    lineCap = cap;
    Impl().set_stroke_caps(BL_STROKE_CAP_SQUARE);
  }
}

std::string Context::GetLineJoin() const {
  switch (context_.stroke_join()) {
  case BL_STROKE_JOIN_ROUND:
    return "round";
  case BL_STROKE_JOIN_BEVEL:
    return "bevel";
  default:
    return "miter";
  }
}

void Context::SetLineJoin(const std::string &join) {
  if (join == "round") {
    lineJoin = join;
    Impl().set_stroke_join(BL_STROKE_JOIN_ROUND);
  } else if (join == "bevel") {
    lineJoin = join;
    Impl().set_stroke_join(BL_STROKE_JOIN_BEVEL);
  } else if (join == "miter") {
    lineJoin = join;
    Impl().set_stroke_join(BL_STROKE_JOIN_MITER_CLIP);
  }
}

double Context::GetMiterLimit() const { return context_.stroke_miter_limit(); }

void Context::SetMiterLimit(double limit) {
  if (std::isfinite(limit) && limit > 0.0) {
    miterLimit = limit;
    Impl().set_stroke_miter_limit(limit);
  }
}

double Context::GetGlobalAlpha() const { return context_.global_alpha(); }

void Context::SetGlobalAlpha(double alpha) {
  if (std::isfinite(alpha) && alpha >= 0.0 && alpha <= 1.0) {
    globalAlpha = alpha;
    Impl().set_global_alpha(alpha);
  }
}

std::string Context::GetGlobalCompositeOperation() const {
  switch (context_.comp_op()) {
  case BL_COMP_OP_SRC_OVER:
    return "source-over";
  case BL_COMP_OP_SRC_IN:
    return "source-in";
  case BL_COMP_OP_SRC_OUT:
    return "source-out";
  case BL_COMP_OP_SRC_ATOP:
    return "source-atop";
  case BL_COMP_OP_DST_OVER:
    return "destination-over";
  case BL_COMP_OP_DST_IN:
    return "destination-in";
  case BL_COMP_OP_DST_OUT:
    return "destination-out";
  case BL_COMP_OP_DST_ATOP:
    return "destination-atop";
  case BL_COMP_OP_PLUS:
    return "lighter";
  case BL_COMP_OP_SRC_COPY:
    return "copy";
  case BL_COMP_OP_XOR:
    return "xor";
  case BL_COMP_OP_MULTIPLY:
    return "multiply";
  case BL_COMP_OP_SCREEN:
    return "screen";
  case BL_COMP_OP_OVERLAY:
    return "overlay";
  case BL_COMP_OP_DARKEN:
    return "darken";
  case BL_COMP_OP_LIGHTEN:
    return "lighten";
  case BL_COMP_OP_COLOR_DODGE:
    return "color-dodge";
  case BL_COMP_OP_COLOR_BURN:
    return "color-burn";
  case BL_COMP_OP_HARD_LIGHT:
    return "hard-light";
  case BL_COMP_OP_SOFT_LIGHT:
    return "soft-light";
  case BL_COMP_OP_DIFFERENCE:
    return "difference";
  case BL_COMP_OP_EXCLUSION:
    return "exclusion";
  default:
    return "source-over";
  }
}

void Context::SetGlobalCompositeOperation(const std::string &op) {
  static const std::unordered_map<std::string, BLCompOp> kCompOps = {
      {"source-over", BL_COMP_OP_SRC_OVER},
      {"source-in", BL_COMP_OP_SRC_IN},
      {"source-out", BL_COMP_OP_SRC_OUT},
      {"source-atop", BL_COMP_OP_SRC_ATOP},
      {"destination-over", BL_COMP_OP_DST_OVER},
      {"destination-in", BL_COMP_OP_DST_IN},
      {"destination-out", BL_COMP_OP_DST_OUT},
      {"destination-atop", BL_COMP_OP_DST_ATOP},
      {"lighter", BL_COMP_OP_PLUS},
      {"copy", BL_COMP_OP_SRC_COPY},
      {"xor", BL_COMP_OP_XOR},
      {"multiply", BL_COMP_OP_MULTIPLY},
      {"screen", BL_COMP_OP_SCREEN},
      {"overlay", BL_COMP_OP_OVERLAY},
      {"darken", BL_COMP_OP_DARKEN},
      {"lighten", BL_COMP_OP_LIGHTEN},
      {"color-dodge", BL_COMP_OP_COLOR_DODGE},
      {"color-burn", BL_COMP_OP_COLOR_BURN},
      {"hard-light", BL_COMP_OP_HARD_LIGHT},
      {"soft-light", BL_COMP_OP_SOFT_LIGHT},
      {"difference", BL_COMP_OP_DIFFERENCE},
      {"exclusion", BL_COMP_OP_EXCLUSION},
  };
  if (auto it = kCompOps.find(op); it != kCompOps.end()) {
    Impl().set_comp_op(it->second);
  }
}

Canvas::Canvas(int width, int height) {
  constexpr int kMaxCanvasDimension = 16384;
  if (width <= 0 || height <= 0 || width > kMaxCanvasDimension ||
      height > kMaxCanvasDimension) {
    throw quickjs::Exception();
  }
  if (surface_.create(width, height, BL_FORMAT_PRGB32) != BL_SUCCESS) {
    throw quickjs::Exception();
  }
  BLContext(surface_).clear_all();
}

BLImage &Canvas::Impl() { return surface_; }

const Context Canvas::CreateContext() { return Context(BLContext(surface_)); }

const char *Canvas::GetAttribute([[maybe_unused]] const char *attr) {
  return "nope";
}

bool Canvas::HasAttribute([[maybe_unused]] const char *attr) { return false; }

void Canvas::Write(std::filesystem::path &png) {
  if (surface_.write_to_file(png.c_str()) != BL_SUCCESS) {
    throw quickjs::Exception();
  }
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
                     .Name("lineWidth")
                     .Getter<GetLineWidth>()
                     .Setter<SetLineWidth>()
                     .Build())
          .Proto(EntryBuilder<canvas::Context>()
                     .Name("lineCap")
                     .Getter<GetLineCap>()
                     .Setter<SetLineCap>()
                     .Build())
          .Proto(EntryBuilder<canvas::Context>()
                     .Name("lineJoin")
                     .Getter<GetLineJoin>()
                     .Setter<SetLineJoin>()
                     .Build())
          .Proto(EntryBuilder<canvas::Context>()
                     .Name("miterLimit")
                     .Getter<GetMiterLimit>()
                     .Setter<SetMiterLimit>()
                     .Build())
          .Proto(EntryBuilder<canvas::Context>()
                     .Name("lineDashOffset")
                     .Getter<GetLineDashOffset>()
                     .Setter<SetLineDashOffset>()
                     .Build())
          .Proto(EntryBuilder<canvas::Context>()
                     .Name("globalAlpha")
                     .Getter<GetGlobalAlpha>()
                     .Setter<SetGlobalAlpha>()
                     .Build())
          .Proto(EntryBuilder<canvas::Context>()
                     .Name("globalCompositeOperation")
                     .Getter<GetGlobalCompositeOperation>()
                     .Setter<SetGlobalCompositeOperation>()
                     .Build())
          .Proto(EntryBuilder<canvas::Context>()
                     .Name("setLineDash")
                     .Method<SetLineDash>(1)
                     .Build())
          .Proto(EntryBuilder<canvas::Context>()
                     .Name("getLineDash")
                     .Method<GetLineDash>(0)
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
