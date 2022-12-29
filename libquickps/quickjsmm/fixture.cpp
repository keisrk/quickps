#include <libquickps/quickjsmm/esm.hpp>
#include <libquickps/quickjsmm/facade.hpp>
#include <libquickps/quickjsmm/fixture.hpp>

namespace quickps {
namespace quickjs {

using namespace test;
using namespace test::facade;

template <>
std::unique_ptr<Point, OpaqueDeleter<Point>>
New<Point>(Context &ctx, ValueIter first, ValueIter last) {
  auto x = first++->Get<double>(ctx);
  auto y = first++->Get<double>(ctx);

  if (first != last) {
    throw Exception();
  }

  return Runtime::GetInstance().Ctor<Point>(x, y);
}

template <> const Class &GetClass<Point>() {
  static Class kClass =
      ClassBuilder<Point>("Point")
          .Constructor<New<Point>>(2)
          .Proto(EntryBuilder<Point>()
                     .Name("x")
                     .Getter<get_x>()
                     .Setter<set_x>()
                     .Build())
          .Proto(EntryBuilder<Point>()
                     .Name("y")
                     .Getter<get_y>()
                     .Setter<set_y>()
                     .Build())
          .Proto(EntryBuilder<Point>().Name("norm").Method<norm>(0).Build())
          .Build();

  return kClass;
}

template <>
std::unique_ptr<Edge, OpaqueDeleter<Edge>> New<Edge>(Context &, ValueIter first,
                                                     ValueIter last) {
  first++; //->GetOpaque<Point>(ctx);
  first++; //->GetOpaque<Point>(ctx);

  if (first != last) {
    throw Exception();
  }

  throw Exception();
}

} // namespace quickjs
} // namespace quickps
