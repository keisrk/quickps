#include <libquickps/quickjsmm/fixture.hpp>

namespace test {
namespace facade {

using namespace quickps::quickjs;

std::unique_ptr<Point, OpaqueDeleter<Point>> ctor(Context &ctx, ValueIter first,
                                                  ValueIter last) {
  auto x = first++->Get<double>(ctx);
  auto y = first++->Get<double>(ctx);

  if (first != last) {
    throw Exception();
  }

  return Runtime::GetInstance().Ctor<Point>(x, y);
}

Value norm(Context &ctx, Point *p, ValueIter first, ValueIter last) {
  if (first != last)
    throw Exception();

  return ctx.Get(p->norm());
}

Value get_x(Context &ctx, Point *p) { return ctx.Get(p->x); }

void set_x(Context &ctx, Point *p, Value v) { return ctx.Set(p->x, v); }

Value get_y(Context &ctx, Point *p) { return ctx.Get(p->y); }

void set_y(Context &ctx, Point *p, Value v) { return ctx.Set(p->y, v); }

} // namespace facade
} // namespace test
