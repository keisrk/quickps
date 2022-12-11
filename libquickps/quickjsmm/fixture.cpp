#include <libquickps/quickjsmm/fixture.hpp>

namespace test {
namespace facade {

using namespace quickps::quickjs;

std::unique_ptr<Point, OpaqueDeleter<Point>> ctor(Context &ctx, ValueIter first,
                                                  ValueIter last) {
  double fst, snd;
  ctx.Set(fst, *first++);
  ctx.Set(snd, *first++);

  if (first != last) {
    throw Exception();
  }

  return Runtime::Ctor<Point>(fst, snd);
}

Value norm(Context &ctx, Point *p, ValueIter first, ValueIter last) {
  if (first != last)
    throw Exception();
  double result = p->norm();
  return ctx.Get(result);
}

Value get_x(Context &ctx, Point *p) { return ctx.Get(p->x); }

void set_x(Context &ctx, Point *p, Value v) { return ctx.Set(p->x, v); }

Value get_y(Context &ctx, Point *p) { return ctx.Get(p->y); }

void set_y(Context &ctx, Point *p, Value v) { return ctx.Set(p->y, v); }

} // namespace facade
} // namespace test
