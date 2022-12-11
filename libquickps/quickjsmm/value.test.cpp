#include <gtest/gtest.h>
#include <libquickps/quickjsmm/value.hpp>
#include <vector>

namespace {

using namespace quickps::quickjs;

class QuickJsValueTest : public ::testing::Test {
protected:
  QuickJsValueTest() : rt_(JS_NewRuntime()), ctx_(JS_NewContext(this->rt_)) {}
  virtual void SetUp() override {
    js_values_[0] = JS_NewBool(ctx_, true);
    js_values_[1] = JS_NewInt32(ctx_, 42);
    js_values_[2] = JS_NewInt64(ctx_, 42);
    js_values_[3] = JS_NewFloat64(ctx_, 3.14);
    js_values_[4] = JS_NewString(ctx_, "Hello QuickJS");
  }
  /*
      if (JS_VALUE_HAS_REF_COUNT(v)) {
          JSRefCountHeader *p = (JSRefCountHeader *)JS_VALUE_GET_PTR(v);
          if (--p->ref_count <= 0) {
  */
  JSRuntime *rt_;
  JSContext *ctx_;
  JSValue js_values_[5];
  size_t kArgCount = 5;
};

TEST_F(QuickJsValueTest, ArgumentListIsIteratable) {
  ArgumentList args(kArgCount, js_values_);
  std::vector<Value> values(args.begin(), args.end());

  EXPECT_EQ(values.size(), kArgCount);

  int i = 0;
  for (auto v : values) {
    EXPECT_EQ(v, Value(js_values_[i++]));
  }
}
} // namespace
