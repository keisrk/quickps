#include <gtest/gtest.h>
#include <libquickps/quickjsmm/context.hpp>
#include <libquickps/quickjsmm/esm.hpp>
#include <libquickps/quickjsmm/fixture.hpp>

namespace {

using namespace quickps::quickjs;
using namespace test;

class QuickJsEsmTest : public ::testing::Test {
protected:
  void SetUp() override {}
};

TEST(QuickJsEsmTest, AmendMeLater) {
  auto cls = ClassSpec<Point>("Point")
                 .Constructor<facade::ctor>()
                 .Proto(EntryBuilder<Point>()
                            .Name("x")
                            .Getter<facade::get_x>()
                            .Setter<facade::set_x>()
                            .Build())
                 .Proto(EntryBuilder<Point>()
                            .Name("y")
                            .Getter<facade::get_y>()
                            .Setter<facade::set_y>()
                            .Build())
                 .Proto(EntryBuilder<Point>()
                            .Name("norm")
                            .Method<facade::norm>(0)
                            .Build());
  EXPECT_NE(&cls, nullptr);
}

TEST(QuickJsEsmTest, AmendMeLaterLater) {
  const auto &ctx = Runtime::CreateContext();
  EXPECT_NE(ctx, Runtime::CreateContext());
}

TEST(QuickJsEsmTest, AmmendMeLaterLaterLater) {
  const auto &ctx = Runtime::CreateContext();
  EXPECT_NE(ctx, Runtime::CreateContext());
}

TEST(QuickJsEsmTest, AmmendMeLaterLaterLaterLater) {
  //  auto *p_ptr = Runtime::Ctor<Point>(1.0, 2.0);
  //  auto *e_ptr = Runtime::Ctor<Edge>();
  //  Runtime::Dtor<Point>(p_ptr);
  //  Runtime::Dtor<Edge>(e_ptr);
}

} // namespace
