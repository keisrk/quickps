#include <gtest/gtest.h>
#include <libquickps/canvas/context.hpp>

namespace {
using namespace quickps::canvas;
namespace qjs = quickps::quickjs;

class CanvasRenderingContext2dTest : public ::testing::Test {
protected:
  void SetUp() override { qjs::Runtime::Init(); }
  void TearDown() override { qjs::Runtime::Terminate(); }
};

TEST_F(CanvasRenderingContext2dTest, CairommApi) {
  auto canvas = Canvas(600, 480);
  const auto &ctx = canvas.CreateContext();
  EXPECT_NE(&ctx, nullptr);
}

TEST_F(CanvasRenderingContext2dTest, CreateCanvasJSInterop) {
  const auto &ctx = qjs::Runtime::CreateContext();
  auto canvas = ctx->Register<Canvas>(qjs::GetClass<Canvas>());
  auto create_canvas = ctx->Register(Canvas::CreateCanvas());
  JSValue args[3];
  args[0] = ctx->Get(640).cobj();
  args[1] = ctx->Get(480).cobj();
  args[2] = ctx->Get("image").cobj();
  auto canvas_instance =
      qjs::RcValue(*ctx, JS_Call(ctx->cobj(), create_canvas.cobj(),
                                 qjs::Value::GetNull().cobj(), 3, &args[0]));
  auto is_instanceof_canvas =
      JS_IsInstanceOf(ctx->cobj(), canvas_instance.cobj(), canvas.cobj());
  EXPECT_TRUE(is_instanceof_canvas);
}

TEST_F(CanvasRenderingContext2dTest, GetContextJSInterop) {
  const auto &ctx = qjs::Runtime::CreateContext();
  auto canvas = ctx->Register<Canvas>(qjs::GetClass<Canvas>());
  auto context = ctx->Register<Context>(qjs::GetClass<Context>());

  EXPECT_TRUE(JS_IsConstructor(ctx->cobj(), canvas.cobj()));
  EXPECT_TRUE(JS_IsConstructor(ctx->cobj(), context.cobj()));

  JSValue args[3];
  args[0] = ctx->Get(640).cobj();
  args[1] = ctx->Get(480).cobj();
  args[2] = ctx->Get("image").cobj();

  auto canvas_instance = qjs::RcValue(
      *ctx, JS_CallConstructor(ctx->cobj(), canvas.cobj(), 3, &args[0]));
  EXPECT_FALSE(canvas_instance.IsException());

  std::vector<qjs::Value> input{ctx->Get("2d")};
  auto context_instance =
      qjs::RcValue(*ctx, canvas_instance.Invoke("getContext", input));

  auto is_instanceof_context =
      JS_IsInstanceOf(ctx->cobj(), context_instance.cobj(), context.cobj());
  EXPECT_TRUE(is_instanceof_context);

  auto ret = context_instance.Invoke("save", {});
  EXPECT_FALSE(ret.IsException());
  EXPECT_TRUE(ret.IsUndefined());
  ret = context_instance.Invoke("restore", {});
  EXPECT_FALSE(ret.IsException());
  EXPECT_TRUE(ret.IsUndefined());
}

} // namespace
