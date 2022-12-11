#include <array>
#include <gtest/gtest.h>
#include <libquickps/quickjsmm/context.hpp>
#include <paper.h>

namespace {

TEST(PaperJsTest, PaperJsGetByteCode) {
  const auto &buf = paper::GetByteCode();
  EXPECT_EQ(buf.size(), 0x59e51);
}

TEST(PaperJsTest, LoadByteCode) {
  const auto &buf = paper::GetByteCode();
  const auto &ctx = quickps::quickjs::Runtime::CreateContext();
  auto result = ctx->LoadModule<paper::ByteCode>(buf);
  EXPECT_FALSE(result.IsException());
}

} // namespace
