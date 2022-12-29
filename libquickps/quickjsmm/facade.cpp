#include <libquickps/quickjsmm/facade.hpp>

namespace test {
namespace facade {

using namespace quickps::quickjs;

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
