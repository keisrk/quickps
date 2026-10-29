#include <cstdio>
#include <iostream>
#include <libquickps/canvas/facade.hpp>
#include <libquickps/logger.hpp>

namespace quickps {
namespace canvas {
namespace facade {

namespace qjs = quickps::quickjs;

namespace {

void EnsureStyleClassRegistered(qjs::Context &ctx) {
  auto class_id = qjs::Runtime::GetInstance().ClassId<Style>();
  JSRuntime *rt = JS_GetRuntime(ctx.cobj());
  if (!JS_IsRegisteredClass(rt, class_id)) {
    JSClassDef def{};
    def.class_name = "CanvasStyle";
    def.finalizer = [](JSRuntime *, JSValue val) {
      qjs::Runtime::GetInstance().Finalize<Style>(qjs::Value(val));
    };
    qjs::Runtime::GetInstance().Register<Style>(def);
  }
}

JSValue AddColorStop(JSContext *js_ctx, JSValueConst this_val, int argc,
                     JSValueConst *argv) {
  try {
    if (argc < 2) {
      throw qjs::Exception();
    }
    qjs::Context ctx(js_ctx);
    auto *style = ctx.GetOpaque<Style>(qjs::Value(this_val));
    double offset = qjs::Value(argv[0]).Get<double>(ctx);
    auto color_str = qjs::Value(argv[1]).Get<std::string>(ctx);
    auto color = Color::FromString(color_str);
    if (auto *grad = std::get_if<BLGradient>(&style->value)) {
      grad->add_stop(offset,
                     BLRgba(color.r, color.g, color.b, color.a).to_rgba32());
    }
    return qjs::Value::GetUndefined().cobj();
  } catch (const std::exception &e) {
    return JS_ThrowInternalError(js_ctx, "%s", e.what());
  } catch (...) {
    return JS_ThrowInternalError(js_ctx, "Unknown C++ exception");
  }
}

qjs::Value CreateStyleObject(qjs::Context &ctx, Style style) {
  EnsureStyleClassRegistered(ctx);
  bool is_gradient = std::holds_alternative<BLGradient>(style.value);
  auto ptr = qjs::Runtime::GetInstance().Ctor<Style>(std::move(style));
  auto obj = ctx.CreateOpaque<Style>(std::move(ptr), qjs::Value::GetNull());
  if (is_gradient) {
    JSValue fn = JS_NewCFunction(ctx.cobj(), AddColorStop, "addColorStop", 2);
    JS_SetPropertyStr(ctx.cobj(), obj.cobj(), "addColorStop", fn);
  }
  return obj;
}

qjs::Value StyleToJSValue(qjs::Context &ctx, const Style &style) {
  if (const auto *color = std::get_if<Color>(&style.value)) {
    BLRgba rgba(color->r, color->g, color->b, color->a);
    BLRgba32 rgba32 = rgba.to_rgba32();
    char buf[64];
    if (rgba32.a() == 255) {
      std::snprintf(buf, sizeof(buf), "#%02x%02x%02x", rgba32.r(), rgba32.g(),
                    rgba32.b());
    } else {
      std::snprintf(buf, sizeof(buf), "rgba(%u, %u, %u, %g)", rgba32.r(),
                    rgba32.g(), rgba32.b(), static_cast<double>(rgba.a));
    }
    return ctx.Get(buf);
  }
  return CreateStyleObject(ctx, style);
}

} // namespace

qjs::Value GetFillStyle(qjs::Context &ctx, Context *c2d) {
  return StyleToJSValue(ctx, c2d->GetFillStyle());
}

void SetFillStyle(qjs::Context &ctx, Context *c2d, qjs::Value value) {
  if (value.InstanceOf<std::string>()) {
    auto css_color = value.Get<std::string>(ctx);
    auto color = Color::FromString(css_color);
    c2d->SetFillStyle(color);
    JS_DupValue(ctx.cobj(), value.cobj());
  } else if (value.InstanceOf<std::unordered_map<std::string, qjs::Value>>()) {
    EnsureStyleClassRegistered(ctx);
    if (auto *style = qjs::Runtime::GetInstance().GetOpaque<Style>(value)) {
      c2d->SetFillStyle(*style);
    }
    JS_DupValue(ctx.cobj(), value.cobj());
  } else {
    JS_DupValue(ctx.cobj(), value.cobj());
  }
}

qjs::Value GetStrokeStyle(qjs::Context &ctx, Context *c2d) {
  return StyleToJSValue(ctx, c2d->GetStrokeStyle());
}

void SetStrokeStyle(qjs::Context &ctx, Context *c2d, qjs::Value value) {
  if (value.InstanceOf<std::string>()) {
    auto css_color = value.Get<std::string>(ctx);
    auto color = Color::FromString(css_color);
    c2d->SetStrokeStyle(color);
    JS_DupValue(ctx.cobj(), value.cobj());
  } else if (value.InstanceOf<std::unordered_map<std::string, qjs::Value>>()) {
    EnsureStyleClassRegistered(ctx);
    if (auto *style = qjs::Runtime::GetInstance().GetOpaque<Style>(value)) {
      c2d->SetStrokeStyle(*style);
    }
    JS_DupValue(ctx.cobj(), value.cobj());
  } else {
    JS_DupValue(ctx.cobj(), value.cobj());
  }
}

qjs::Value GetLineWidth(qjs::Context &ctx, Context *c2d) {
  return ctx.Get(c2d->GetLineWidth());
}

void SetLineWidth(qjs::Context &ctx, Context *c2d, qjs::Value value) {
  c2d->SetLineWidth(value.Get<double>(ctx));
  JS_DupValue(ctx.cobj(), value.cobj());
}

qjs::Value GetLineCap(qjs::Context &ctx, Context *c2d) {
  return ctx.Get(c2d->GetLineCap());
}

void SetLineCap(qjs::Context &ctx, Context *c2d, qjs::Value value) {
  if (value.InstanceOf<std::string>()) {
    c2d->SetLineCap(value.Get<std::string>(ctx));
  }
  JS_DupValue(ctx.cobj(), value.cobj());
}

qjs::Value GetLineJoin(qjs::Context &ctx, Context *c2d) {
  return ctx.Get(c2d->GetLineJoin());
}

void SetLineJoin(qjs::Context &ctx, Context *c2d, qjs::Value value) {
  if (value.InstanceOf<std::string>()) {
    c2d->SetLineJoin(value.Get<std::string>(ctx));
  }
  JS_DupValue(ctx.cobj(), value.cobj());
}

qjs::Value GetMiterLimit(qjs::Context &ctx, Context *c2d) {
  return ctx.Get(c2d->GetMiterLimit());
}

void SetMiterLimit(qjs::Context &ctx, Context *c2d, qjs::Value value) {
  c2d->SetMiterLimit(value.Get<double>(ctx));
  JS_DupValue(ctx.cobj(), value.cobj());
}

qjs::Value GetLineDashOffset(qjs::Context &ctx, Context *c2d) {
  return ctx.Get(c2d->GetLineDashOffset());
}

void SetLineDashOffset(qjs::Context &ctx, Context *c2d, qjs::Value value) {
  c2d->SetLineDashOffset(value.Get<double>(ctx));
  JS_DupValue(ctx.cobj(), value.cobj());
}

qjs::Value GetGlobalAlpha(qjs::Context &ctx, Context *c2d) {
  return ctx.Get(c2d->GetGlobalAlpha());
}

void SetGlobalAlpha(qjs::Context &ctx, Context *c2d, qjs::Value value) {
  c2d->SetGlobalAlpha(value.Get<double>(ctx));
  JS_DupValue(ctx.cobj(), value.cobj());
}

qjs::Value GetGlobalCompositeOperation(qjs::Context &ctx, Context *c2d) {
  return ctx.Get(c2d->GetGlobalCompositeOperation());
}

void SetGlobalCompositeOperation(qjs::Context &ctx, Context *c2d,
                                 qjs::Value value) {
  if (value.InstanceOf<std::string>()) {
    c2d->SetGlobalCompositeOperation(value.Get<std::string>(ctx));
  }
  JS_DupValue(ctx.cobj(), value.cobj());
}

qjs::Value Save(qjs::Context &, Context *c2d, qjs::ValueIter first,
                qjs::ValueIter last) {
  log::debug("save");

  if (first != last)
    throw qjs::Exception();

  c2d->Save();
  return qjs::Value::GetUndefined();
}

qjs::Value Restore(qjs::Context &, Context *c2d, qjs::ValueIter first,
                   qjs::ValueIter last) {
  log::debug("restore");

  if (first != last)
    throw qjs::Exception();

  c2d->Restore();
  return qjs::Value::GetUndefined();
}

qjs::Value Scale(qjs::Context &ctx, Context *c2d, qjs::ValueIter first,
                 qjs::ValueIter last) {
  auto x = first++->Get<double>(ctx);
  auto y = first++->Get<double>(ctx);

  if (first != last)
    throw qjs::Exception();

  c2d->Scale(x, y);
  return qjs::Value::GetUndefined();
}

qjs::Value Rotate(qjs::Context &ctx, Context *c2d, qjs::ValueIter first,
                  qjs::ValueIter last) {
  auto rad = first++->Get<double>(ctx);

  if (first != last)
    throw qjs::Exception();

  c2d->Rotate(rad);
  return qjs::Value::GetUndefined();
}

qjs::Value Translate(qjs::Context &ctx, Context *c2d, qjs::ValueIter first,
                     qjs::ValueIter last) {
  auto x = first++->Get<double>(ctx);
  auto y = first++->Get<double>(ctx);

  if (first != last)
    throw qjs::Exception();

  c2d->Translate(x, y);
  return qjs::Value::GetUndefined();
}

qjs::Value Transform(qjs::Context &ctx, Context *c2d, qjs::ValueIter first,
                     qjs::ValueIter last) {
  auto a = first++->Get<double>(ctx);
  auto b = first++->Get<double>(ctx);
  auto c = first++->Get<double>(ctx);
  auto d = first++->Get<double>(ctx);
  auto e = first++->Get<double>(ctx);
  auto f = first++->Get<double>(ctx);

  if (first != last)
    throw qjs::Exception();

  c2d->Transform(a, b, c, d, e, f);
  return qjs::Value::GetUndefined();
}

qjs::Value SetTransform(qjs::Context &ctx, Context *c2d, qjs::ValueIter first,
                        qjs::ValueIter last) {
  auto a = first++->Get<double>(ctx);
  auto b = first++->Get<double>(ctx);
  auto c = first++->Get<double>(ctx);
  auto d = first++->Get<double>(ctx);
  auto e = first++->Get<double>(ctx);
  auto f = first++->Get<double>(ctx);

  if (first != last)
    throw qjs::Exception();

  c2d->SetTransform(a, b, c, d, e, f);
  return qjs::Value::GetUndefined();
}

qjs::Value CreateLinearGradient(qjs::Context &ctx, Context *c2d,
                                qjs::ValueIter first, qjs::ValueIter last) {
  auto x0 = first++->Get<double>(ctx);
  auto y0 = first++->Get<double>(ctx);
  auto x1 = first++->Get<double>(ctx);
  auto y1 = first++->Get<double>(ctx);

  if (first != last)
    throw qjs::Exception();

  return CreateStyleObject(ctx, c2d->CreateLinearGradient(x0, y0, x1, y1));
}

qjs::Value CreateRadialGradient(qjs::Context &ctx, Context *c2d,
                                qjs::ValueIter first, qjs::ValueIter last) {
  auto x0 = first++->Get<double>(ctx);
  auto y0 = first++->Get<double>(ctx);
  auto r0 = first++->Get<double>(ctx);
  auto x1 = first++->Get<double>(ctx);
  auto y1 = first++->Get<double>(ctx);
  auto r1 = first++->Get<double>(ctx);

  if (first != last)
    throw qjs::Exception();

  return CreateStyleObject(ctx,
                           c2d->CreateRadialGradient(x0, y0, r0, x1, y1, r1));
}

qjs::Value CreatePattern(qjs::Context &ctx, Context *c2d, qjs::ValueIter first,
                         qjs::ValueIter last) {
  if (first == last)
    throw qjs::Exception();

  auto *image = ctx.GetOpaque<Canvas>(*first++);
  std::string repetition = "repeat";
  if (first != last) {
    if (!first->IsUndefined() && !first->InstanceOf<std::nullptr_t>()) {
      repetition = first->Get<std::string>(ctx);
      if (repetition.empty()) {
        repetition = "repeat";
      }
    }
    ++first;
  }

  if (first != last)
    throw qjs::Exception();

  return CreateStyleObject(ctx, c2d->CreatePattern(*image, repetition));
}

qjs::Value ClearRect(qjs::Context &ctx, Context *c2d, qjs::ValueIter first,
                     qjs::ValueIter last) {
  log::debug("clearRect");

  auto x = first++->Get<double>(ctx);
  auto y = first++->Get<double>(ctx);
  auto width = first++->Get<double>(ctx);
  auto height = first++->Get<double>(ctx);

  if (first != last)
    throw qjs::Exception();

  c2d->ClearRect(x, y, width, height);
  return qjs::Value::GetUndefined();
}

qjs::Value FillRect(qjs::Context &ctx, Context *c2d, qjs::ValueIter first,
                    qjs::ValueIter last) {
  log::debug("fillRect");

  auto x = first++->Get<double>(ctx);
  auto y = first++->Get<double>(ctx);
  auto width = first++->Get<double>(ctx);
  auto height = first++->Get<double>(ctx);

  if (first != last)
    throw qjs::Exception();

  c2d->FillRect(x, y, width, height);
  return qjs::Value::GetUndefined();
}

qjs::Value StrokeRect(qjs::Context &ctx, Context *c2d, qjs::ValueIter first,
                      qjs::ValueIter last) {
  log::debug("strokeRect");

  auto x = first++->Get<double>(ctx);
  auto y = first++->Get<double>(ctx);
  auto width = first++->Get<double>(ctx);
  auto height = first++->Get<double>(ctx);

  if (first != last)
    throw qjs::Exception();

  c2d->StrokeRect(x, y, width, height);
  return qjs::Value::GetUndefined();
}

qjs::Value BeginPath(qjs::Context &, Context *c2d, qjs::ValueIter first,
                     qjs::ValueIter last) {
  log::debug("beginPath");

  if (first != last)
    throw qjs::Exception();

  c2d->BeginPath();
  return qjs::Value::GetUndefined();
}

qjs::Value ClosePath(qjs::Context &, Context *c2d, qjs::ValueIter first,
                     qjs::ValueIter last) {
  log::debug("closePath");

  if (first != last)
    throw qjs::Exception();

  c2d->ClosePath();
  return qjs::Value::GetUndefined();
}

qjs::Value MoveTo(qjs::Context &ctx, Context *c2d, qjs::ValueIter first,
                  qjs::ValueIter last) {
  log::debug("moveTo");

  auto x = first++->Get<double>(ctx);
  auto y = first++->Get<double>(ctx);

  if (first != last)
    throw qjs::Exception();

  c2d->MoveTo(x, y);
  return qjs::Value::GetUndefined();
}

qjs::Value LineTo(qjs::Context &ctx, Context *c2d, qjs::ValueIter first,
                  qjs::ValueIter last) {
  log::debug("lineTo");

  auto x = first++->Get<double>(ctx);
  auto y = first++->Get<double>(ctx);

  if (first != last)
    throw qjs::Exception();

  c2d->LineTo(x, y);
  return qjs::Value::GetUndefined();
}

qjs::Value QuadraticCurveTo(qjs::Context &ctx, Context *c2d,
                            qjs::ValueIter first, qjs::ValueIter last) {
  auto cpx = first++->Get<double>(ctx);
  auto cpy = first++->Get<double>(ctx);
  auto x = first++->Get<double>(ctx);
  auto y = first++->Get<double>(ctx);

  if (first != last)
    throw qjs::Exception();

  c2d->QuadraticCurveTo(cpx, cpy, x, y);
  return qjs::Value::GetUndefined();
}

qjs::Value BezierCurveTo(qjs::Context &ctx, Context *c2d, qjs::ValueIter first,
                         qjs::ValueIter last) {
  log::debug("bezierCurveTo");

  auto cp1x = first++->Get<double>(ctx);
  auto cp1y = first++->Get<double>(ctx);
  auto cp2x = first++->Get<double>(ctx);
  auto cp2y = first++->Get<double>(ctx);
  auto x = first++->Get<double>(ctx);
  auto y = first++->Get<double>(ctx);

  if (first != last)
    throw qjs::Exception();

  c2d->BezierCurveTo(cp1x, cp1y, cp2x, cp2y, x, y);
  return qjs::Value::GetUndefined();
}

qjs::Value ArcTo(qjs::Context &ctx, Context *c2d, qjs::ValueIter first,
                 qjs::ValueIter last) {
  log::debug("arcTo");

  auto x1 = first++->Get<double>(ctx);
  auto y1 = first++->Get<double>(ctx);
  auto x2 = first++->Get<double>(ctx);
  auto y2 = first++->Get<double>(ctx);
  auto radius = first++->Get<double>(ctx);

  if (first != last)
    throw qjs::Exception();

  c2d->ArcTo(x1, y1, x2, y2, radius);
  return qjs::Value::GetUndefined();
}

qjs::Value Rect(qjs::Context &ctx, Context *c2d, qjs::ValueIter first,
                qjs::ValueIter last) {
  log::debug("rect");

  auto x = first++->Get<double>(ctx);
  auto y = first++->Get<double>(ctx);
  auto width = first++->Get<double>(ctx);
  auto height = first++->Get<double>(ctx);

  if (first != last)
    throw qjs::Exception();

  c2d->Rect(x, y, width, height);
  return qjs::Value::GetUndefined();
}

qjs::Value Arc(qjs::Context &ctx, Context *c2d, qjs::ValueIter first,
               qjs::ValueIter last) {
  log::debug("arc");

  auto x = first++->Get<double>(ctx);
  auto y = first++->Get<double>(ctx);
  auto radius = first++->Get<double>(ctx);
  auto start_angle = first++->Get<double>(ctx);
  auto end_angle = first++->Get<double>(ctx);
  bool anticlockwise = false;

  if (first != last) {
    anticlockwise = first++->Get<bool>(ctx);
  }

  if (first != last)
    throw qjs::Exception();

  c2d->Arc(x, y, radius, start_angle, end_angle, anticlockwise);
  return qjs::Value::GetUndefined();
}

qjs::Value Fill(qjs::Context &ctx, Context *c2d, qjs::ValueIter first,
                qjs::ValueIter last) {
  log::debug("fill");

  std::optional<std::string> fill_rule;

  if (first != last)
    fill_rule = first++->Get<std::string>(ctx);

  if (first != last)
    throw qjs::Exception();

  if (fill_rule) {
    c2d->Fill(fill_rule.value());
  } else {
    c2d->Fill();
  }

  return qjs::Value::GetUndefined();
}

qjs::Value Stroke(qjs::Context &, Context *c2d, qjs::ValueIter first,
                  qjs::ValueIter last) {
  log::debug("stroke");

  if (first != last)
    throw qjs::Exception();

  c2d->Stroke();
  return qjs::Value::GetUndefined();
}

qjs::Value Clip(qjs::Context &ctx, Context *c2d, qjs::ValueIter first,
                qjs::ValueIter last) {
  log::debug("clip");

  std::optional<std::string> fill_rule;

  if (first != last)
    fill_rule = first++->Get<std::string>(ctx);

  if (first != last)
    throw qjs::Exception();

  if (fill_rule) {
    c2d->Clip(fill_rule.value());
  } else {
    c2d->Clip();
  }

  return qjs::Value::GetUndefined();
}

qjs::Value IsPointInPath(qjs::Context &ctx, Context *c2d, qjs::ValueIter first,
                         qjs::ValueIter last) {
  log::debug("isPointInPath");

  std::optional<std::string> fill_rule;

  auto x = first++->Get<double>(ctx);
  auto y = first++->Get<double>(ctx);

  if (first != last)
    fill_rule = first++->Get<std::string>(ctx);

  if (first != last)
    throw qjs::Exception();

  bool res = false;
  if (fill_rule) {
    res = c2d->IsPointInPath(x, y, fill_rule.value());
  } else {
    res = c2d->IsPointInPath(x, y);
  }

  return ctx.Get(res);
}

qjs::Value FillText([[maybe_unused]] qjs::Context &ctx,
                    [[maybe_unused]] Context *c2d,
                    [[maybe_unused]] qjs::ValueIter first,
                    [[maybe_unused]] qjs::ValueIter last) {
  throw qjs::Exception();
}
qjs::Value StrokeText([[maybe_unused]] qjs::Context &ctx,
                      [[maybe_unused]] Context *c2d,
                      [[maybe_unused]] qjs::ValueIter first,
                      [[maybe_unused]] qjs::ValueIter last) {
  throw qjs::Exception();
}
qjs::Value MeasureText([[maybe_unused]] qjs::Context &ctx,
                       [[maybe_unused]] Context *c2d,
                       [[maybe_unused]] qjs::ValueIter first,
                       [[maybe_unused]] qjs::ValueIter last) {
  throw qjs::Exception();
}
qjs::Value DrawImage([[maybe_unused]] qjs::Context &ctx,
                     [[maybe_unused]] Context *c2d,
                     [[maybe_unused]] qjs::ValueIter first,
                     [[maybe_unused]] qjs::ValueIter last) {
  throw qjs::Exception();
}
qjs::Value CreateImageData([[maybe_unused]] qjs::Context &ctx,
                           [[maybe_unused]] Context *c2d,
                           [[maybe_unused]] qjs::ValueIter first,
                           [[maybe_unused]] qjs::ValueIter last) {
  throw qjs::Exception();
}
qjs::Value GetImageData([[maybe_unused]] qjs::Context &ctx,
                        [[maybe_unused]] Context *c2d,
                        [[maybe_unused]] qjs::ValueIter first,
                        [[maybe_unused]] qjs::ValueIter last) {
  throw qjs::Exception();
}
qjs::Value PutImageData([[maybe_unused]] qjs::Context &ctx,
                        [[maybe_unused]] Context *c2d,
                        [[maybe_unused]] qjs::ValueIter first,
                        [[maybe_unused]] qjs::ValueIter last) {
  throw qjs::Exception();
}
qjs::Value SetLineDash(qjs::Context &ctx, Context *c2d, qjs::ValueIter first,
                       qjs::ValueIter last) {
  if (first == last)
    throw qjs::Exception();

  auto arr = *first++;
  if (first != last)
    throw qjs::Exception();

  if (!JS_IsArray(ctx.cobj(), arr.cobj()))
    throw qjs::Exception();

  JSValue len_val = JS_GetPropertyStr(ctx.cobj(), arr.cobj(), "length");
  uint32_t len = 0;
  JS_ToUint32(ctx.cobj(), &len, len_val);
  JS_FreeValue(ctx.cobj(), len_val);

  std::vector<double> segments;
  segments.reserve(len);
  for (uint32_t i = 0; i < len; ++i) {
    JSValue elem = JS_GetPropertyUint32(ctx.cobj(), arr.cobj(), i);
    double val = 0.0;
    int err = JS_ToFloat64(ctx.cobj(), &val, elem);
    JS_FreeValue(ctx.cobj(), elem);
    if (err != 0)
      throw qjs::Exception();
    segments.push_back(val);
  }

  c2d->SetLineDash(segments);
  return qjs::Value::GetUndefined();
}

qjs::Value GetLineDash(qjs::Context &ctx, Context *c2d, qjs::ValueIter first,
                       qjs::ValueIter last) {
  if (first != last)
    throw qjs::Exception();

  auto segments = c2d->GetLineDash();
  JSValue arr = JS_NewArray(ctx.cobj());
  for (uint32_t i = 0; i < segments.size(); ++i) {
    JS_SetPropertyUint32(ctx.cobj(), arr, i,
                         JS_NewFloat64(ctx.cobj(), segments[i]));
  }
  return qjs::Value(arr);
}

qjs::Value CreateCanvas(qjs::Context &ctx,
                        [[maybe_unused]] qjs::Value this_value,
                        qjs::ValueIter first, qjs::ValueIter last) {
  log::debug("createCanvas");
  auto ptr = quickjs::New<Canvas>(ctx, first, last);
  auto canvas = qjs::RcValue(
      ctx, ctx.CreateOpaque<Canvas>(std::move(ptr), qjs::Value::GetNull()));

  return canvas.Take().cobj();
}

qjs::Value GetContext(qjs::Context &ctx, Canvas *canvas,
                      [[maybe_unused]] qjs::ValueIter first,
                      [[maybe_unused]] qjs::ValueIter last) {
  log::debug("getContext");

  auto ptr = qjs::Runtime::GetInstance().Ctor<Context>(canvas->CreateContext());
  return ctx.CreateOpaque<Context>(std::move(ptr), qjs::Value::GetNull())
      .cobj();
}

qjs::Value get_width(qjs::Context &ctx, Canvas *canvas) {
  return ctx.Get(canvas->Impl().width());
}

void set_width(qjs::Context &, Canvas *, qjs::Value) {}

qjs::Value get_height(qjs::Context &ctx, Canvas *canvas) {
  return ctx.Get(canvas->Impl().height());
}

void set_height(qjs::Context &, Canvas *, qjs::Value) {}

qjs::Value get_type(qjs::Context &ctx, Canvas *) { return ctx.Get("image"); }

void set_type(qjs::Context &, Canvas *, qjs::Value) {}

qjs::Value GetAttribute(qjs::Context &ctx, Canvas *canvas, qjs::ValueIter first,
                        qjs::ValueIter last) {
  auto attr = first++->Get<std::string>(ctx);

  if (first != last)
    throw qjs::Exception();

  auto v = canvas->GetAttribute(attr.c_str());
  return ctx.Get(v);
}

qjs::Value HasAttribute(qjs::Context &ctx, Canvas *canvas, qjs::ValueIter first,
                        qjs::ValueIter last) {
  auto attr = first++->Get<std::string>(ctx);

  if (first != last)
    throw qjs::Exception();

  auto v = canvas->HasAttribute(attr.c_str());
  return ctx.Get(v);
}

qjs::Value AddEventListener([[maybe_unused]] qjs::Context &ctx,
                            [[maybe_unused]] Canvas *canvas,
                            [[maybe_unused]] qjs::ValueIter first,
                            [[maybe_unused]] qjs::ValueIter last) {
  return qjs::Value::GetUndefined();
}

qjs::Value GetStyle(qjs::Context &ctx, [[maybe_unused]] Canvas *canvas) {
  return ctx.GetObject({}).Take();
}

void SetStyle([[maybe_unused]] qjs::Context &ctx,
              [[maybe_unused]] Canvas *canvas, [[maybe_unused]] qjs::Value v) {
  throw Unimplemented();
}

qjs::Value GetOwnerDocument([[maybe_unused]] qjs::Context &ctx,
                            [[maybe_unused]] Canvas *canvas) {
  return ctx.GetGlobalThis().Get("self").Get("document").Take();
}

void SetOwnerDocument([[maybe_unused]] qjs::Context &ctx,
                      [[maybe_unused]] Canvas *canvas,
                      [[maybe_unused]] qjs::Value v) {
  throw Unimplemented();
}

} // namespace facade
} // namespace canvas
} // namespace quickps
