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

TEST_F(CanvasRenderingContext2dTest, PathAndGeometryJSInterop) {
  const auto &ctx = qjs::Runtime::CreateContext();
  auto canvas = ctx->Register<Canvas>(qjs::GetClass<Canvas>());
  ctx->Register<Context>(qjs::GetClass<Context>());

  JSValue args[3] = {ctx->Get(100).cobj(), ctx->Get(100).cobj(),
                     ctx->Get("image").cobj()};
  auto canvas_instance = qjs::RcValue(
      *ctx, JS_CallConstructor(ctx->cobj(), canvas.cobj(), 3, &args[0]));
  auto c2d = qjs::RcValue(
      *ctx, canvas_instance.Invoke("getContext", {ctx->Get("2d")}));

  // Test arc and isPointInPath returning bool
  c2d.Invoke("beginPath", {});
  c2d.Invoke("arc", {ctx->Get(50.0), ctx->Get(50.0), ctx->Get(25.0),
                     ctx->Get(0.0), ctx->Get(6.283185307179586)});
  auto inside = qjs::RcValue(
      *ctx, c2d.Invoke("isPointInPath", {ctx->Get(50.0), ctx->Get(50.0)}));
  EXPECT_TRUE(inside.value().InstanceOf<bool>());
  EXPECT_TRUE(inside.value().Get<bool>(*ctx));

  auto outside = qjs::RcValue(
      *ctx, c2d.Invoke("isPointInPath", {ctx->Get(5.0), ctx->Get(5.0)}));
  EXPECT_TRUE(outside.value().InstanceOf<bool>());
  EXPECT_FALSE(outside.value().Get<bool>(*ctx));

  // Test arcTo
  c2d.Invoke("beginPath", {});
  c2d.Invoke("moveTo", {ctx->Get(10.0), ctx->Get(10.0)});
  auto arc_to_ret =
      c2d.Invoke("arcTo", {ctx->Get(90.0), ctx->Get(10.0), ctx->Get(90.0),
                           ctx->Get(90.0), ctx->Get(20.0)});
  EXPECT_FALSE(arc_to_ret.IsException());
  EXPECT_TRUE(arc_to_ret.IsUndefined());
}

TEST_F(CanvasRenderingContext2dTest, StrokeAndLineDashJSInterop) {
  const auto &ctx = qjs::Runtime::CreateContext();
  auto canvas = ctx->Register<Canvas>(qjs::GetClass<Canvas>());
  ctx->Register<Context>(qjs::GetClass<Context>());

  JSValue args[3] = {ctx->Get(100).cobj(), ctx->Get(100).cobj(),
                     ctx->Get("image").cobj()};
  auto canvas_instance = qjs::RcValue(
      *ctx, JS_CallConstructor(ctx->cobj(), canvas.cobj(), 3, &args[0]));
  auto c2d = qjs::RcValue(
      *ctx, canvas_instance.Invoke("getContext", {ctx->Get("2d")}));

  c2d.Set("lineWidth", ctx->Get(4.5));
  EXPECT_DOUBLE_EQ(c2d.Get("lineWidth").value().Get<double>(*ctx), 4.5);

  c2d.Set("lineCap", ctx->Get("round"));
  EXPECT_EQ(c2d.Get("lineCap").value().Get<std::string>(*ctx), "round");

  c2d.Set("lineJoin", ctx->Get("bevel"));
  EXPECT_EQ(c2d.Get("lineJoin").value().Get<std::string>(*ctx), "bevel");

  c2d.Set("miterLimit", ctx->Get(8.0));
  EXPECT_DOUBLE_EQ(c2d.Get("miterLimit").value().Get<double>(*ctx), 8.0);

  c2d.Set("lineDashOffset", ctx->Get(2.5));
  EXPECT_DOUBLE_EQ(c2d.Get("lineDashOffset").value().Get<double>(*ctx), 2.5);

  JSValue dash_arr = JS_NewArray(ctx->cobj());
  JS_SetPropertyUint32(ctx->cobj(), dash_arr, 0,
                       JS_NewFloat64(ctx->cobj(), 5.0));
  JS_SetPropertyUint32(ctx->cobj(), dash_arr, 1,
                       JS_NewFloat64(ctx->cobj(), 10.0));
  qjs::RcValue dash_rc(*ctx, dash_arr);
  auto set_ret = c2d.Invoke("setLineDash", {dash_rc.value()});
  EXPECT_FALSE(set_ret.IsException());

  auto got_dash = qjs::RcValue(*ctx, c2d.Invoke("getLineDash", {}));
  EXPECT_TRUE(JS_IsArray(ctx->cobj(), got_dash.cobj()));
  JSValue elem0 = JS_GetPropertyUint32(ctx->cobj(), got_dash.cobj(), 0);
  JSValue elem1 = JS_GetPropertyUint32(ctx->cobj(), got_dash.cobj(), 1);
  EXPECT_DOUBLE_EQ(qjs::Value(elem0).Get<double>(*ctx), 5.0);
  EXPECT_DOUBLE_EQ(qjs::Value(elem1).Get<double>(*ctx), 10.0);
  JS_FreeValue(ctx->cobj(), elem0);
  JS_FreeValue(ctx->cobj(), elem1);
}

TEST_F(CanvasRenderingContext2dTest, StylesAndCompositingJSInterop) {
  const auto &ctx = qjs::Runtime::CreateContext();
  auto canvas = ctx->Register<Canvas>(qjs::GetClass<Canvas>());
  ctx->Register<Context>(qjs::GetClass<Context>());

  JSValue args[3] = {ctx->Get(100).cobj(), ctx->Get(100).cobj(),
                     ctx->Get("image").cobj()};
  auto canvas_instance = qjs::RcValue(
      *ctx, JS_CallConstructor(ctx->cobj(), canvas.cobj(), 3, &args[0]));
  auto c2d = qjs::RcValue(
      *ctx, canvas_instance.Invoke("getContext", {ctx->Get("2d")}));

  // Default fillStyle and strokeStyle
  EXPECT_EQ(c2d.Get("fillStyle").value().Get<std::string>(*ctx), "#000000");
  EXPECT_EQ(c2d.Get("strokeStyle").value().Get<std::string>(*ctx), "#000000");

  c2d.Set("fillStyle", ctx->Get("#ff0000"));
  EXPECT_EQ(c2d.Get("fillStyle").value().Get<std::string>(*ctx), "#ff0000");

  c2d.Set("strokeStyle", ctx->Get("rgba(0, 255, 0, 128)"));
  EXPECT_EQ(c2d.Get("strokeStyle").value().Get<std::string>(*ctx),
            "rgba(0, 255, 0, 0.501961)");

  // globalAlpha and globalCompositeOperation
  c2d.Set("globalAlpha", ctx->Get(0.75));
  EXPECT_DOUBLE_EQ(c2d.Get("globalAlpha").value().Get<double>(*ctx), 0.75);

  c2d.Set("globalCompositeOperation", ctx->Get("multiply"));
  EXPECT_EQ(c2d.Get("globalCompositeOperation").value().Get<std::string>(*ctx),
            "multiply");

  // Linear gradient
  auto lin_grad =
      qjs::RcValue(*ctx, c2d.Invoke("createLinearGradient",
                                    {ctx->Get(0.0), ctx->Get(0.0),
                                     ctx->Get(100.0), ctx->Get(0.0)}));
  EXPECT_FALSE(lin_grad.IsException());
  lin_grad.Invoke("addColorStop", {ctx->Get(0.0), ctx->Get("#ff0000")});
  lin_grad.Invoke("addColorStop", {ctx->Get(1.0), ctx->Get("#0000ff")});
  c2d.Set("fillStyle", lin_grad.Take());
  c2d.Set("globalAlpha", ctx->Get(1.0));
  c2d.Set("globalCompositeOperation", ctx->Get("source-over"));
  c2d.Invoke("fillRect",
             {ctx->Get(0.0), ctx->Get(0.0), ctx->Get(100.0), ctx->Get(100.0)});

  // Radial gradient
  auto rad_grad = qjs::RcValue(
      *ctx, c2d.Invoke("createRadialGradient",
                       {ctx->Get(50.0), ctx->Get(50.0), ctx->Get(5.0),
                        ctx->Get(50.0), ctx->Get(50.0), ctx->Get(50.0)}));
  EXPECT_FALSE(rad_grad.IsException());
  rad_grad.Invoke("addColorStop", {ctx->Get(0.0), ctx->Get("#ffffff")});
  rad_grad.Invoke("addColorStop", {ctx->Get(1.0), ctx->Get("#000000")});
  c2d.Set("strokeStyle", rad_grad.Take());

  // Pattern
  auto pat =
      qjs::RcValue(*ctx, c2d.Invoke("createPattern", {canvas_instance.value(),
                                                      ctx->Get("repeat")}));
  EXPECT_FALSE(pat.IsException());
  c2d.Set("fillStyle", pat.Take());
}

} // namespace
