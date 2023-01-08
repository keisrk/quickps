#include <iostream>
#include <libquickps/canvas/facade.hpp>

namespace quickps {
namespace canvas {
namespace facade {

namespace qjs = quickps::quickjs;

qjs::Value GetFillStyle([[maybe_unused]] qjs::Context &ctx,
                        [[maybe_unused]] Context *c2d) {
  return qjs::Value::GetUndefined();
}

void SetFillStyle(qjs::Context &ctx, [[maybe_unused]] Context *c2d,
                  qjs::Value value) {
  if (value.InstanceOf<std::string>()) {
    [[maybe_unused]] auto color = value.Get<std::string>(ctx);
    JS_DupValue(ctx.cobj(), value.cobj());
  } else if (value.InstanceOf<std::unordered_map<std::string, qjs::Value>>()) {
    [[maybe_unused]] auto pattern =
        value.Get<std::unordered_map<std::string, qjs::Value>>(ctx);
    JS_DupValue(ctx.cobj(), value.cobj());
  } else {
    JS_DupValue(ctx.cobj(), value.cobj());
  }
}

qjs::Value GetStrokeStyle([[maybe_unused]] qjs::Context &ctx,
                          [[maybe_unused]] Context *c2d) {
  return qjs::Value::GetUndefined();
}

void SetStrokeStyle(qjs::Context &ctx, [[maybe_unused]] Context *c2d,
                    qjs::Value value) {
  if (value.InstanceOf<std::string>()) {
    [[maybe_unused]] auto color = value.Get<std::string>(ctx);
    JS_DupValue(ctx.cobj(), value.cobj());
  } else if (value.InstanceOf<std::unordered_map<std::string, qjs::Value>>()) {
    [[maybe_unused]] auto pattern =
        value.Get<std::unordered_map<std::string, qjs::Value>>(ctx);
    JS_DupValue(ctx.cobj(), value.cobj());
  } else {
    JS_DupValue(ctx.cobj(), value.cobj());
  }
}

qjs::Value Save(qjs::Context &, Context *c2d, qjs::ValueIter first,
                qjs::ValueIter last) {
  std::cout << "save\n";

  if (first != last)
    throw qjs::Exception();

  c2d->Save();
  return qjs::Value::GetUndefined();
}

qjs::Value Restore(qjs::Context &, Context *c2d, qjs::ValueIter first,
                   qjs::ValueIter last) {
  std::cout << "restore\n";

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

qjs::Value CreateLinearGradient([[maybe_unused]] qjs::Context &ctx,
                                [[maybe_unused]] Context *c2d,
                                [[maybe_unused]] qjs::ValueIter first,
                                [[maybe_unused]] qjs::ValueIter last) {
  throw qjs::Exception();
}

qjs::Value CreateRadialGradient([[maybe_unused]] qjs::Context &ctx,
                                [[maybe_unused]] Context *c2d,
                                [[maybe_unused]] qjs::ValueIter first,
                                [[maybe_unused]] qjs::ValueIter last) {
  throw qjs::Exception();
}

qjs::Value CreatePattern([[maybe_unused]] qjs::Context &ctx,
                         [[maybe_unused]] Context *c2d,
                         [[maybe_unused]] qjs::ValueIter first,
                         [[maybe_unused]] qjs::ValueIter last) {
  throw qjs::Exception();
}

qjs::Value ClearRect(qjs::Context &ctx, Context *c2d, qjs::ValueIter first,
                     qjs::ValueIter last) {
  std::cout << "clearRect\n";

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
  std::cout << "fillRect\n";

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
  std::cout << "strokeRect\n";

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
  std::cout << "beginPath\n";

  if (first != last)
    throw qjs::Exception();

  c2d->BeginPath();
  return qjs::Value::GetUndefined();
}

qjs::Value ClosePath(qjs::Context &, Context *c2d, qjs::ValueIter first,
                     qjs::ValueIter last) {
  std::cout << "closePath\n";

  if (first != last)
    throw qjs::Exception();

  c2d->ClosePath();
  return qjs::Value::GetUndefined();
}

qjs::Value MoveTo(qjs::Context &ctx, Context *c2d, qjs::ValueIter first,
                  qjs::ValueIter last) {
  std::cout << "moveTo\n";

  auto x = first++->Get<double>(ctx);
  auto y = first++->Get<double>(ctx);

  if (first != last)
    throw qjs::Exception();

  c2d->MoveTo(x, y);
  return qjs::Value::GetUndefined();
}

qjs::Value LineTo(qjs::Context &ctx, Context *c2d, qjs::ValueIter first,
                  qjs::ValueIter last) {
  std::cout << "lineTo\n";

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
  std::cout << "bezierCurveTo\n";

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

qjs::Value ArcTo([[maybe_unused]] qjs::Context &ctx,
                 [[maybe_unused]] Context *c2d,
                 [[maybe_unused]] qjs::ValueIter first,
                 [[maybe_unused]] qjs::ValueIter last) {
  std::cout << "arcTo\n";
  throw qjs::Exception();
}

qjs::Value Rect(qjs::Context &ctx, Context *c2d, qjs::ValueIter first,
                qjs::ValueIter last) {
  std::cout << "rect\n";

  auto x = first++->Get<double>(ctx);
  auto y = first++->Get<double>(ctx);
  auto width = first++->Get<double>(ctx);
  auto height = first++->Get<double>(ctx);

  if (first != last)
    throw qjs::Exception();

  c2d->Rect(x, y, width, height);
  return qjs::Value::GetUndefined();
}

qjs::Value Arc([[maybe_unused]] qjs::Context &ctx,
               [[maybe_unused]] Context *c2d,
               [[maybe_unused]] qjs::ValueIter first,
               [[maybe_unused]] qjs::ValueIter last) {
  std::cout << "arc\n";
  throw qjs::Exception();
}

qjs::Value Fill(qjs::Context &ctx, Context *c2d, qjs::ValueIter first,
                qjs::ValueIter last) {
  std::cout << "fill\n";

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
  std::cout << "stroke\n";

  if (first != last)
    throw qjs::Exception();

  c2d->Stroke();
  return qjs::Value::GetUndefined();
}

qjs::Value Clip(qjs::Context &ctx, Context *c2d, qjs::ValueIter first,
                qjs::ValueIter last) {
  std::cout << "clip\n";

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
  std::cout << "isPointInPath\n";

  std::optional<std::string> fill_rule;

  auto x = first++->Get<double>(ctx);
  auto y = first++->Get<double>(ctx);

  if (first != last)
    fill_rule = first++->Get<std::string>(ctx);

  if (first != last)
    throw qjs::Exception();

  if (fill_rule) {
    c2d->IsPointInPath(x, y, fill_rule.value());
  } else {
    c2d->IsPointInPath(x, y);
  }

  return qjs::Value::GetUndefined();
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
qjs::Value SetLineDash([[maybe_unused]] qjs::Context &ctx,
                       [[maybe_unused]] Context *c2d,
                       [[maybe_unused]] qjs::ValueIter first,
                       [[maybe_unused]] qjs::ValueIter last) {
  throw qjs::Exception();
}

qjs::Value CreateCanvas(qjs::Context &ctx,
                        [[maybe_unused]] qjs::Value this_value,
                        qjs::ValueIter first, qjs::ValueIter last) {
  std::cout << "createCavnas\n";
  auto ptr = quickjs::New<Canvas>(ctx, first, last);
  auto canvas = qjs::RcValue(
      ctx, ctx.CreateOpaque<Canvas>(std::move(ptr), qjs::Value::GetNull()));

  return canvas.Take().cobj();
}

qjs::Value GetContext(qjs::Context &ctx, Canvas *canvas,
                      [[maybe_unused]] qjs::ValueIter first,
                      [[maybe_unused]] qjs::ValueIter last) {
  std::cout << "getContext\n";

  auto ptr = qjs::Runtime::GetInstance().Ctor<Context>(canvas->CreateContext());
  return ctx.CreateOpaque<Context>(std::move(ptr), qjs::Value::GetNull())
      .cobj();
}

qjs::Value get_width(qjs::Context &ctx, Canvas *canvas) {
  return ctx.Get(canvas->Impl()->get_width());
}

void set_width(qjs::Context &, Canvas *, qjs::Value) {}

qjs::Value get_height(qjs::Context &ctx, Canvas *canvas) {
  return ctx.Get(canvas->Impl()->get_height());
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
