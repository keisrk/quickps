#ifndef QUICKPS_QUICKJSMM_FIXTURE_H_
#define QUICKPS_QUICKJSMM_FIXTURE_H_
#include <cmath>
#include <libquickps/quickjsmm/context.hpp>

namespace test {

struct Point {
  double x;
  double y;
  Point() = default;
  Point(double x, double y) : x(x), y(y) {}
  double norm() { return sqrt(pow(x, 2) + pow(y, 2)); }
};

class Edge {
public:
  Point start_;
  Point end_;
};

namespace facade {

using namespace quickps::quickjs;
std::unique_ptr<Point, OpaqueDeleter<Point>> ctor(Context &ctx, ValueIter first,
                                                  ValueIter last);
Value norm(Context &ctx, Point *p, ValueIter first, ValueIter last);
Value get_x(Context &ctx, Point *p);
void set_x(Context &ctx, Point *p, Value v);
Value get_y(Context &ctx, Point *p);
void set_y(Context &ctx, Point *p, Value v);

} // namespace facade
} // namespace test

#endif // QUICKPS_QUICKJSMM_FIXTURE_H_
