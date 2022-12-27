#include <gtest/gtest.h>
#include <libquickps/canvas/rendering_context_2d.hpp>

namespace {

using namespace quickps::canvas;

TEST(CanvasRenderingContext2dTest, AmendMeLater) {
  auto canvas = Canvas(600, 480);
  const auto &ctx = canvas.CreateContext();
}

TEST(CanvasRenderingContext2dTest, AmendMeLaterLater) {}

} // namespace
