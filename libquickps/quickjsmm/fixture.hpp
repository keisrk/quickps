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

} // namespace test

namespace quickps {
namespace quickjs {

template <>
std::unique_ptr<test::Point, OpaqueDeleter<test::Point>>
New<test::Point>(Context &ctx, ValueIter first, ValueIter last);

template <> const Class &GetClass<test::Point>();

template <>
std::unique_ptr<test::Edge, OpaqueDeleter<test::Edge>>
New<test::Edge>(Context &ctx, ValueIter first, ValueIter last);

template <> const Class &GetClass<test::Edge>();

} // namespace quickjs
} // namespace quickps

#endif // QUICKPS_QUICKJSMM_FIXTURE_H_
