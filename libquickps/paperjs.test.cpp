#include <array>
#include <gtest/gtest.h>
#include <libquickps/quickjsmm/context.hpp>
#include <paper.h>

namespace {

class PaperJsTest : public ::testing::Test {
protected:
  void SetUp() override { quickps::quickjs::Runtime::Init(); }
  void TearDown() override { quickps::quickjs::Runtime::Terminate(); }
};

TEST_F(PaperJsTest, PaperJsGetByteCode) {
  const auto &buf = paper::GetByteCode();
  EXPECT_EQ(buf.size(), 0x59e51);
}

TEST_F(PaperJsTest, LoadByteCode) {
  const auto &buf = paper::GetByteCode();
  const auto &ctx = quickps::quickjs::Runtime::CreateContext();
  auto result = ctx->LoadModule<paper::ByteCode>(buf);
  EXPECT_FALSE(result.IsException());
  EXPECT_EQ(JS_VALUE_GET_TAG(result.value()), JS_TAG_MODULE);
  JSModuleDef *m = static_cast<JSModuleDef *>(result.value().u.ptr);
  EXPECT_NE(m, nullptr);
  auto atom = JS_GetModuleName(ctx->GetInstance(), m);
  const char *module_name = JS_AtomToCString(ctx->GetInstance(), atom);
  EXPECT_STREQ(module_name, "paper.js");
  auto val = quickps::quickjs::Value(
      JS_EvalFunction(ctx->GetInstance(), result.value()));
  EXPECT_FALSE(val.IsException());
}

} // namespace
