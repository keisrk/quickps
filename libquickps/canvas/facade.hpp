#ifndef QUICKPS_CANVAS_FACADE_H_
#define QUICKPS_CANVAS_FACADE_H_

#include <libquickps/canvas/context.hpp>

namespace quickps {
namespace canvas {
namespace facade {

namespace qjs = quickps::quickjs;
// Context
qjs::Value GetFillStyle(qjs::Context &ctx, Context *c2d);
void SetFillStyle(qjs::Context &, Context *c2d, qjs::Value value);
qjs::Value GetStrokeStyle(qjs::Context &ctx, Context *c2d);
void SetStrokeStyle(qjs::Context &, Context *c2d, qjs::Value value);
qjs::Value GetLineWidth(qjs::Context &ctx, Context *c2d);
void SetLineWidth(qjs::Context &ctx, Context *c2d, qjs::Value value);
qjs::Value GetLineCap(qjs::Context &ctx, Context *c2d);
void SetLineCap(qjs::Context &ctx, Context *c2d, qjs::Value value);
qjs::Value GetLineJoin(qjs::Context &ctx, Context *c2d);
void SetLineJoin(qjs::Context &ctx, Context *c2d, qjs::Value value);
qjs::Value GetMiterLimit(qjs::Context &ctx, Context *c2d);
void SetMiterLimit(qjs::Context &ctx, Context *c2d, qjs::Value value);
qjs::Value GetLineDashOffset(qjs::Context &ctx, Context *c2d);
void SetLineDashOffset(qjs::Context &ctx, Context *c2d, qjs::Value value);
qjs::Value GetGlobalAlpha(qjs::Context &ctx, Context *c2d);
void SetGlobalAlpha(qjs::Context &ctx, Context *c2d, qjs::Value value);
qjs::Value GetGlobalCompositeOperation(qjs::Context &ctx, Context *c2d);
void SetGlobalCompositeOperation(qjs::Context &ctx, Context *c2d,
                                 qjs::Value value);
qjs::Value GetFont(qjs::Context &ctx, Context *c2d);
void SetFont(qjs::Context &ctx, Context *c2d, qjs::Value value);
qjs::Value GetTextAlign(qjs::Context &ctx, Context *c2d);
void SetTextAlign(qjs::Context &ctx, Context *c2d, qjs::Value value);
qjs::Value GetTextBaseline(qjs::Context &ctx, Context *c2d);
void SetTextBaseline(qjs::Context &ctx, Context *c2d, qjs::Value value);
qjs::Value GetDirection(qjs::Context &ctx, Context *c2d);
void SetDirection(qjs::Context &ctx, Context *c2d, qjs::Value value);
qjs::Value GetLetterSpacing(qjs::Context &ctx, Context *c2d);
void SetLetterSpacing(qjs::Context &ctx, Context *c2d, qjs::Value value);
qjs::Value GetLineDash(qjs::Context &ctx, Context *c2d, qjs::ValueIter first,
                       qjs::ValueIter last);
qjs::Value Save(qjs::Context &ctx, Context *c2d, qjs::ValueIter first,
                qjs::ValueIter last);
qjs::Value Restore(qjs::Context &ctx, Context *c2d, qjs::ValueIter first,
                   qjs::ValueIter last);
qjs::Value Scale(qjs::Context &ctx, Context *c2d, qjs::ValueIter first,
                 qjs::ValueIter last);
qjs::Value Rotate(qjs::Context &ctx, Context *c2d, qjs::ValueIter first,
                  qjs::ValueIter last);
qjs::Value Translate(qjs::Context &ctx, Context *c2d, qjs::ValueIter first,
                     qjs::ValueIter last);
qjs::Value Transform(qjs::Context &ctx, Context *c2d, qjs::ValueIter first,
                     qjs::ValueIter last);
qjs::Value SetTransform(qjs::Context &ctx, Context *c2d, qjs::ValueIter first,
                        qjs::ValueIter last);
qjs::Value CreateLinearGradient(qjs::Context &ctx, Context *c2d,
                                qjs::ValueIter first, qjs::ValueIter last);
qjs::Value CreateRadialGradient(qjs::Context &ctx, Context *c2d,
                                qjs::ValueIter first, qjs::ValueIter last);
qjs::Value CreatePattern(qjs::Context &ctx, Context *c2d, qjs::ValueIter first,
                         qjs::ValueIter last);
qjs::Value ClearRect(qjs::Context &ctx, Context *c2d, qjs::ValueIter first,
                     qjs::ValueIter last);
qjs::Value FillRect(qjs::Context &ctx, Context *c2d, qjs::ValueIter first,
                    qjs::ValueIter last);
qjs::Value StrokeRect(qjs::Context &ctx, Context *c2d, qjs::ValueIter first,
                      qjs::ValueIter last);
qjs::Value BeginPath(qjs::Context &ctx, Context *c2d, qjs::ValueIter first,
                     qjs::ValueIter last);
qjs::Value ClosePath(qjs::Context &ctx, Context *c2d, qjs::ValueIter first,
                     qjs::ValueIter last);
qjs::Value MoveTo(qjs::Context &ctx, Context *c2d, qjs::ValueIter first,
                  qjs::ValueIter last);
qjs::Value LineTo(qjs::Context &ctx, Context *c2d, qjs::ValueIter first,
                  qjs::ValueIter last);
qjs::Value QuadraticCurveTo(qjs::Context &ctx, Context *c2d,
                            qjs::ValueIter first, qjs::ValueIter last);
qjs::Value BezierCurveTo(qjs::Context &ctx, Context *c2d, qjs::ValueIter first,
                         qjs::ValueIter last);
qjs::Value ArcTo(qjs::Context &ctx, Context *c2d, qjs::ValueIter first,
                 qjs::ValueIter last);
qjs::Value Rect(qjs::Context &ctx, Context *c2d, qjs::ValueIter first,
                qjs::ValueIter last);
qjs::Value Arc(qjs::Context &ctx, Context *c2d, qjs::ValueIter first,
               qjs::ValueIter last);
qjs::Value Fill(qjs::Context &ctx, Context *c2d, qjs::ValueIter first,
                qjs::ValueIter last);
qjs::Value Stroke(qjs::Context &ctx, Context *c2d, qjs::ValueIter first,
                  qjs::ValueIter last);
qjs::Value Clip(qjs::Context &ctx, Context *c2d, qjs::ValueIter first,
                qjs::ValueIter last);
qjs::Value IsPointInPath(qjs::Context &ctx, Context *c2d, qjs::ValueIter first,
                         qjs::ValueIter last);
qjs::Value FillText(qjs::Context &ctx, Context *c2d, qjs::ValueIter first,
                    qjs::ValueIter last);
qjs::Value StrokeText(qjs::Context &ctx, Context *c2d, qjs::ValueIter first,
                      qjs::ValueIter last);
qjs::Value MeasureText(qjs::Context &ctx, Context *c2d, qjs::ValueIter first,
                       qjs::ValueIter last);
qjs::Value DrawImage(qjs::Context &ctx, Context *c2d,
                     const qjs::ArgumentList &args);
qjs::Value CreateImageData(qjs::Context &ctx, Context *c2d,
                           const qjs::ArgumentList &args);
qjs::Value GetImageData(qjs::Context &ctx, Context *c2d,
                        const qjs::ArgumentList &args);
qjs::Value PutImageData(qjs::Context &ctx, Context *c2d,
                        const qjs::ArgumentList &args);
qjs::Value GetImageSmoothingEnabled(qjs::Context &ctx, Context *c2d);
void SetImageSmoothingEnabled(qjs::Context &ctx, Context *c2d, qjs::Value val);
qjs::Value GetImageSmoothingQuality(qjs::Context &ctx, Context *c2d);
void SetImageSmoothingQuality(qjs::Context &ctx, Context *c2d, qjs::Value val);
qjs::Value SetLineDash(qjs::Context &ctx, Context *c2d, qjs::ValueIter first,
                       qjs::ValueIter last);
// Canvas
qjs::Value CreateCanvas(qjs::Context &ctx, qjs::Value this_value,
                        const qjs::ArgumentList &args);
qjs::Value GetContext(qjs::Context &ctx, qjs::Value this_val, Canvas *canvas,
                      const qjs::ArgumentList &args);
qjs::Value get_width(qjs::Context &ctx, Canvas *canvas);
void set_width(qjs::Context &ctx, Canvas *canvas, qjs::Value val);
qjs::Value get_height(qjs::Context &ctx, Canvas *canvas);
void set_height(qjs::Context &ctx, Canvas *canvas, qjs::Value val);
qjs::Value get_type(qjs::Context &ctx, Canvas *canvas);
void set_type(qjs::Context &, Canvas *, qjs::Value);
qjs::Value ToDataURL(qjs::Context &ctx, Canvas *canvas,
                     const qjs::ArgumentList &args);
qjs::Value Write(qjs::Context &ctx, Canvas *canvas,
                 const qjs::ArgumentList &args);

qjs::Value GetAttribute(qjs::Context &ctx, Canvas *canvas, qjs::ValueIter first,
                        qjs::ValueIter last);
qjs::Value HasAttribute(qjs::Context &ctx, Canvas *canvas, qjs::ValueIter first,
                        qjs::ValueIter last);
qjs::Value AddEventListener(qjs::Context &ctx, Canvas *canvas,
                            qjs::ValueIter first, qjs::ValueIter last);

qjs::Value GetStyle(qjs::Context &ctx, Canvas *canvas);
void SetStyle(qjs::Context &ctx, Canvas *canvas, qjs::Value v);
qjs::Value GetOwnerDocument(qjs::Context &ctx, Canvas *canvas);
void SetOwnerDocument(qjs::Context &ctx, Canvas *canvas, qjs::Value v);

}  // namespace facade
}  // namespace canvas
}  // namespace quickps

#endif  // QUICKPS_CANVAS_FACADE_H_
