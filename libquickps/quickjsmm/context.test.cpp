#include <gtest/gtest.h>
#include <libquickps/quickjsmm/context.hpp>
#include <libquickps/quickjsmm/fixture.hpp>

namespace {

using namespace quickps::quickjs;
using namespace test;

class QuickJsContextTest : public ::testing::Test {
protected:
  void SetUp() override {}
};

TEST(QuickJsContextTest, RuntimeIsSingleton) {
  auto &rt1 = Runtime::GetInstance();
  auto &rt2 = Runtime::GetInstance();
  auto *rt1_ptr = &rt1;
  auto *rt2_ptr = &rt2;
  EXPECT_EQ(rt1_ptr, rt2_ptr);
}

TEST(QuickJsContextTest, GetSet) {
  const auto &ctx = Runtime::CreateContext();
  bool b_raw = false;
  int i_raw = 0;
  double d_raw = 0.0;
  std::string s_raw("__EMPTY__");

  auto b_js = ctx->Get(true);
  ctx->Set(b_raw, b_js);
  EXPECT_EQ(b_raw, true);

  auto i_js = ctx->Get(42);
  ctx->Set(i_raw, i_js);
  EXPECT_EQ(i_raw, 42);

  auto d_js = ctx->Get(1.412);
  ctx->Set(d_raw, d_js);
  EXPECT_EQ(d_raw, 1.412);

  auto s_js = ctx->Get("Hello QuickJS");
  ctx->Set(s_raw, s_js);
  EXPECT_EQ(s_raw, "Hello QuickJS");
}

TEST(QuickJsContextTest, AmmendMeLater) {
  const auto &ctx = Runtime::CreateContext();
  auto b_js = ctx->Get(true);
  EXPECT_FALSE(b_js.IsException());
}

TEST(QuickJsContextTest, AmmendMeLaterLater) {
  std::unique_ptr<Point, OpaqueDeleter<Point>> p_ptr =
      Runtime::Ctor<Point>(1.0, 2.0);
  std::unique_ptr<Edge, OpaqueDeleter<Edge>> e_ptr = Runtime::Ctor<Edge>();
  Runtime::Dtor<Point>(p_ptr.release());
  Runtime::Dtor<Edge>(e_ptr.release());

  auto p_id = Runtime::ClassId<Point>();
  auto e_id = Runtime::ClassId<Edge>();
  EXPECT_NE(p_id, e_id);
  EXPECT_EQ(p_id, Runtime::ClassId<Point>());
  EXPECT_EQ(e_id, Runtime::ClassId<Edge>());
}

TEST(QuickJsContextTest, AmmendMeLaterLaterLater) {
  const auto &ctx = Runtime::CreateContext();
  auto point_proto = Value(JS_NewObject(ctx->GetInstance()));
  std::unique_ptr<Point, OpaqueDeleter<Point>> p_ptr =
      Runtime::Ctor<Point>(1.0, 2.0);
  Point *raw_ptr = p_ptr.get();
  Value opaque = ctx->CreateOpaque<Point>(std::move(p_ptr), point_proto);
  Point *opaque_ptr = ctx->GetOpaque<Point>(opaque);
  EXPECT_EQ(raw_ptr, opaque_ptr);
  Runtime::Dtor<Point>(opaque_ptr);
}
} // namespace
