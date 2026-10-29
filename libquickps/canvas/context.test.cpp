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

TEST_F(CanvasRenderingContext2dTest, Blend2dApi) {
  auto canvas = Canvas(600, 480);
  const auto &ctx = canvas.CreateContext();
  EXPECT_NE(&ctx, nullptr);
}

TEST_F(CanvasRenderingContext2dTest, ClipShim) {
  auto canvas = Canvas(100, 100);
  auto ctx = canvas.CreateContext();

  ctx.Save();
  ctx.BeginPath();
  ctx.MoveTo(50, 20);
  ctx.LineTo(80, 80);
  ctx.LineTo(20, 80);
  ctx.ClosePath();
  ctx.Clip();

  ctx.SetFillStyle(Color(1.0f, 0.0f, 0.0f, 1.0f));
  ctx.FillRect(0, 0, 100, 100);
  ctx.Restore();

  BLImageData data{};
  ASSERT_EQ(canvas.Impl().get_data(&data), BL_SUCCESS);
  const auto *pixels = static_cast<const uint32_t *>(data.pixel_data);
  const auto stride = data.stride / sizeof(uint32_t);

  // Inside triangle (50, 50) should be red (0xFFFF0000 in PRGB32)
  EXPECT_EQ(pixels[50 * stride + 50], 0xFFFF0000U);
  // Outside triangle (10, 10) should be transparent (0x00000000)
  EXPECT_EQ(pixels[10 * stride + 10], 0x00000000U);

  // After Restore(), clipping should be removed
  ctx.SetFillStyle(Color(0.0f, 1.0f, 0.0f, 1.0f));
  ctx.FillRect(0, 0, 20, 20);
  ASSERT_EQ(canvas.Impl().get_data(&data), BL_SUCCESS);
  pixels = static_cast<const uint32_t *>(data.pixel_data);
  EXPECT_EQ(pixels[10 * stride + 10], 0xFF00FF00U);
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
