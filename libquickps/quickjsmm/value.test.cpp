#include <gtest/gtest.h>
#include <libquickps/quickjsmm/value.hpp>
#include <vector>

namespace {

using namespace quickps::quickjs;

class QuickJsValueTest : public ::testing::Test, public ContextProvider {
protected:
  QuickJsValueTest() : rt_(JS_NewRuntime()), ctx_(JS_NewContext(this->rt_)) {}
  virtual void SetUp() override {
    js_values_[0] = JS_NewBool(ctx_, true);
    js_values_[1] = JS_NewInt32(ctx_, 42);
    js_values_[2] = JS_NewInt64(ctx_, 42);
    js_values_[3] = JS_NewFloat64(ctx_, 3.14);
    js_values_[4] = JS_NewString(ctx_, "Hello QuickJS");
  }

  virtual void TearDown() override {
    JS_FreeContext(ctx_);
    JS_FreeRuntime(rt_);
  }

  JSContext *cobj() { return ctx_; }

  JSRuntime *rt_;
  JSContext *ctx_;
  JSValue js_values_[5];
  size_t kArgCount = 5;
};

TEST_F(QuickJsValueTest, ValueWrapsJSValue) {
  EXPECT_EQ(sizeof(JSValue), sizeof(Value));
}

TEST_F(QuickJsValueTest, ArgumentListIsIteratable) {
  ArgumentList args(kArgCount, js_values_);
  std::vector<Value> values(args.begin(), args.end());

  EXPECT_EQ(values.size(), kArgCount);

  int i = 0;
  for (auto v : values) {
    EXPECT_EQ(v, Value(js_values_[i++]));
  }
}

TEST_F(QuickJsValueTest, TypedGetters) {
  ContextProvider &ctx = *this;
  ArgumentList args(kArgCount, js_values_);
  auto it = args.begin();
  EXPECT_TRUE(it++->Get<bool>(ctx));
  EXPECT_EQ(it++->Get<int>(ctx), 42);
  EXPECT_EQ(it++->Get<int>(ctx), 42);
  EXPECT_EQ(it++->Get<double>(ctx), 3.14);
  EXPECT_EQ(it++->Get<std::string>(ctx), "Hello QuickJS");
  EXPECT_EQ(it, args.end());
}

TEST_F(QuickJsValueTest, RcValueAndString) {
  ContextProvider &ctx = *this;
  std::string json(R"({ "width": 600, "height": 480, "type": "image" })");
  auto canvas = RcValue(
      ctx, JS_ParseJSON(ctx.cobj(), json.c_str(), json.size(), "anonymous"));
  auto type =
      RcValue(ctx, JS_GetPropertyStr(ctx.cobj(), canvas.cobj(), "type"));
  auto cstr = RcString(ctx, JS_ToCString(ctx.cobj(), type.cobj()));
  EXPECT_STREQ(cstr.cobj(), "image");
}

} // namespace
