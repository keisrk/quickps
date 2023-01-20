#include <gtest/gtest.h>
#include <libquickps/canvas/color.hpp>
#include <regex>
#include <string>

namespace {
using namespace quickps::canvas;

TEST(ColorTest, CssParser) {
  auto black = Color::FromString("black");
  auto red = Color::FromString("red");
  auto green = Color::FromString("green");
  auto blue = Color::FromString("blue");
  EXPECT_EQ(black, Color::FromString("rgba(0,0,0,255)"));
  EXPECT_EQ(red, Color::FromString("rgb(255,0,0)"));
  EXPECT_EQ(green, Color::FromString("hsla(120,100%,25%,255)"));
  EXPECT_EQ(blue, Color::FromString("hsl(240,100%,50%)"));
}

} // namespace
