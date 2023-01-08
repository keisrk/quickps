#include <gtest/gtest.h>
#include <libquickps/canvas/context.hpp>

namespace {

using namespace quickps::canvas;

class CairoMmTest : public ::testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(CairoMmTest, AllTheWay) {}

} // namespace
