#include <gtest/gtest.h>
#include <libquickps/quickjsmm/context.hpp>
#include <libquickps/quickjsmm/fixture.hpp>

namespace {

using namespace quickps::quickjs;
using namespace test;

class QuickJsContextTest : public ::testing::Test {
protected:
  void SetUp() override { Runtime::Init(); }
  void TearDown() override { Runtime::Terminate(); }
};

TEST_F(QuickJsContextTest, RuntimeIsNotSingleton) {
  auto rt1 = Runtime::GetInstance();
  auto rt2 = Runtime::GetInstance();
  auto *rt1_ptr = &rt1;
  auto *rt2_ptr = &rt2;
  EXPECT_NE(rt1_ptr, rt2_ptr);
}

TEST_F(QuickJsContextTest, GetSet) {
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

TEST_F(QuickJsContextTest, GetterHappyPath) {
  const auto &ctx = Runtime::CreateContext();
  auto b_js = ctx->Get(true);
  EXPECT_FALSE(b_js.IsException());
}

TEST_F(QuickJsContextTest, CtorDtor) {
  std::unique_ptr<Point, OpaqueDeleter<Point>> p_ptr =
      Runtime::GetInstance().Ctor<Point>(1.0, 2.0);
  std::unique_ptr<Edge, OpaqueDeleter<Edge>> e_ptr =
      Runtime::GetInstance().Ctor<Edge>();
  Runtime::Dtor<Point>(p_ptr.release());
  Runtime::Dtor<Edge>(e_ptr.release());

  auto p_id = Runtime::GetInstance().ClassId<Point>();
  auto e_id = Runtime::GetInstance().ClassId<Edge>();
  EXPECT_NE(p_id, e_id);
  EXPECT_EQ(p_id, Runtime::GetInstance().ClassId<Point>());
  EXPECT_EQ(e_id, Runtime::GetInstance().ClassId<Edge>());
}

TEST_F(QuickJsContextTest, GetSetOpaque) {
  const auto &ctx = Runtime::GetInstance().CreateContext();
  std::unique_ptr<Point, OpaqueDeleter<Point>> p_ptr =
      Runtime::GetInstance().Ctor<Point>(1.0, 2.0);

  // Boilerplate
  JSClassDef def;
  def.class_name = "Point";
  def.finalizer = [](auto, auto js_val) {
    Value v(js_val);
    Runtime::GetInstance().Finalize<Point>(v);
  };
  Runtime::GetInstance().Register<Point>(def);

  JSClassID class_id = Runtime::GetInstance().ClassId<Point>();
  EXPECT_TRUE(JS_IsRegisteredClass(JS_GetRuntime(ctx->cobj()), class_id));

  // Assign the Class ID to an opaque object on construction.
  auto opaque_obj = Value(JS_NewObjectClass(ctx->cobj(), class_id));

  JS_SetOpaque(opaque_obj.cobj(), p_ptr.get());
  Point *opaque_ptr = ctx->GetOpaque<Point>(opaque_obj);
  EXPECT_EQ(p_ptr.release(), opaque_ptr); // Prevent double free of p_ptr

  // Free resources
  JS_FreeValue(ctx->cobj(), opaque_obj.cobj());
}

TEST_F(QuickJsContextTest, PointClassConstruction) {
  const auto &ctx = Runtime::GetInstance().CreateContext();
  const auto &spec = GetClass<Point>();
  auto ctor = ctx->Register<Point>(spec);

  EXPECT_TRUE(JS_IsConstructor(ctx->cobj(), ctor.cobj()));

  JSValue argv[2];
  argv[0] = ctx->Get(cos(60 * M_PI / 180)).cobj();
  argv[1] = ctx->Get(sin(60 * M_PI / 180)).cobj();
  auto instance =
      Value(JS_CallConstructor(ctx->cobj(), ctor.cobj(), 2, &argv[0]));
  EXPECT_FALSE(instance.IsException());

  auto *ptr = ctx->GetOpaque<Point>(instance);
  EXPECT_DOUBLE_EQ(ptr->x, cos(60 * M_PI / 180));
  EXPECT_DOUBLE_EQ(ptr->y, sin(60 * M_PI / 180));
  EXPECT_DOUBLE_EQ(ptr->norm(), 1);

  auto norm_ident = JS_NewAtom(ctx->cobj(), "norm");
  EXPECT_TRUE(JS_HasProperty(ctx->cobj(), instance.cobj(), norm_ident));
  auto norm =
      Value(JS_Invoke(ctx->cobj(), instance.cobj(), norm_ident, 0, nullptr));
  EXPECT_EQ(norm.Get<double>(*ctx), 1);

  // Free resources
  JS_FreeValue(ctx->cobj(), instance.cobj());
}

} // namespace
