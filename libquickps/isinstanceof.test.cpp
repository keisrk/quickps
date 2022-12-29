#include <gtest/gtest.h>
#include <libquickps/quickjsmm/context.hpp>
#include <libquickps/quickjsmm/fixture.hpp>
namespace {

using namespace quickps::quickjs;
using namespace test;

class IsInstanceOfTest : public ::testing::Test {
protected:
  void SetUp() override { Runtime::Init(); }
  void TearDown() override { Runtime::Terminate(); }
};

TEST_F(IsInstanceOfTest, AllTheWay) {
  const auto &ctx = Runtime::GetInstance().CreateContext();
  auto ctor = ctx->Register<Point>(GetClass<Point>());
  EXPECT_TRUE(JS_IsConstructor(ctx->cobj(), ctor.cobj()));

  {
    std::vector<JSValue> argv({
        ctx->Get(60.0).cobj(),
        ctx->Get(60.0).cobj(),
    });
    auto instance = RcValue(*ctx, JS_CallConstructor(ctx->cobj(), ctor.cobj(),
                                                     argv.size(), argv.data()));
    EXPECT_FALSE(instance.IsException());

    auto is_instanceof =
        JS_IsInstanceOf(ctx->cobj(), instance.cobj(), ctor.cobj());
    EXPECT_TRUE(is_instanceof);
  }

  {
    auto is_instanceof =
        JS_IsInstanceOf(ctx->cobj(), Value::GetNull().cobj(), ctor.cobj());
    EXPECT_FALSE(is_instanceof);
  }
}

} // namespace
