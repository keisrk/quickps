#ifndef QUICKPS_QUICKJSMM_FACADE_H_
#define QUICKPS_QUICKJSMM_FACADE_H_

#include <libquickps/quickjsmm/fixture.hpp>

namespace test {
namespace facade {

using namespace quickps::quickjs;

Value norm(Context &ctx, Point *p, ValueIter first, ValueIter last);
Value get_x(Context &ctx, Point *p);
void set_x(Context &ctx, Point *p, Value v);
Value get_y(Context &ctx, Point *p);
void set_y(Context &ctx, Point *p, Value v);

} // namespace facade
} // namespace test

#endif // QUICKPS_QUICKJSMM_FACADE_H_
