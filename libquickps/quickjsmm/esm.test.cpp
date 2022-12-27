#include <gtest/gtest.h>
#include <libquickps/quickjsmm/context.hpp>
#include <libquickps/quickjsmm/esm.hpp>
#include <libquickps/quickjsmm/fixture.hpp>

namespace {

using namespace quickps::quickjs;
using namespace test;

class QuickJsEsmTest : public ::testing::Test {
protected:
  void SetUp() override { Runtime::Init(); }
  void TearDown() override { Runtime::Terminate(); }
};

TEST_F(QuickJsEsmTest, AmendMeLater) {
  const auto &ctx = Runtime::CreateContext();
  auto cls = ClassSpec<Point>("Point")
                 .Constructor<facade::ctor>(2)
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
  Value v = cls.Register(*ctx);
  EXPECT_NE(&v, nullptr);
  JS_FreeValue(ctx->GetInstance(), v.value());
}
} // namespace
