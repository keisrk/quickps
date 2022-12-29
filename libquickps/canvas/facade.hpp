#ifndef QUICKPS_CANVAS_FACADE_H_
#define QUICKPS_CANVAS_FACADE_H_

#include <libquickps/canvas/context.hpp>

namespace quickps {
namespace canvas {
namespace facade {

namespace qjs = quickps::quickjs;
// Context
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
qjs::Value DrawImage(qjs::Context &ctx, Context *c2d, qjs::ValueIter first,
                     qjs::ValueIter last);
qjs::Value CreateImageData(qjs::Context &ctx, Context *c2d,
                           qjs::ValueIter first, qjs::ValueIter last);
qjs::Value GetImageData(qjs::Context &ctx, Context *c2d, qjs::ValueIter first,
                        qjs::ValueIter last);
qjs::Value PutImageData(qjs::Context &ctx, Context *c2d, qjs::ValueIter first,
                        qjs::ValueIter last);
qjs::Value SetLineDash(qjs::Context &ctx, Context *c2d, qjs::ValueIter first,
                       qjs::ValueIter last);
// Canvas
qjs::Value CreateCanvas(qjs::Context &ctx, qjs::Value this_value,
                        qjs::ValueIter first, qjs::ValueIter last);
qjs::Value GetContext(qjs::Context &ctx, Canvas *canvas, qjs::ValueIter first,
                      qjs::ValueIter last);
qjs::Value get_width(qjs::Context &ctx, Canvas *canvas);
void set_width(qjs::Context &, Canvas *, qjs::Value);
qjs::Value get_height(qjs::Context &ctx, Canvas *canvas);
void set_height(qjs::Context &, Canvas *, qjs::Value);
qjs::Value get_type(qjs::Context &ctx, Canvas *);
void set_type(qjs::Context &, Canvas *, qjs::Value);

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

} // namespace facade
} // namespace canvas
} // namespace quickps

#endif // QUICKPS_CANVAS_FACADE_H_
