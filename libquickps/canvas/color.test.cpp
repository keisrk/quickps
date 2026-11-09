#include <gtest/gtest.h>

#include <libquickps/canvas/color.hpp>
#include <string>

namespace {
using namespace quickps::canvas;

TEST(ColorTest, CssParserBasic) {
  auto black = Color::FromString("black");
  auto red = Color::FromString("red");
  auto green = Color::FromString("green");
  auto blue = Color::FromString("blue");
  EXPECT_EQ(black, Color::FromString("rgba(0,0,0,1)"));
  EXPECT_EQ(red, Color::FromString("rgb(255,0,0)"));
  EXPECT_EQ(green, Color::FromString("hsla(120,100%,25%,1)"));
  EXPECT_EQ(blue, Color::FromString("hsl(240,100%,50%)"));
}

TEST(ColorTest, HexFormats) {
  EXPECT_EQ(Color::FromString("#f00"), Color(1.0f, 0.0f, 0.0f, 1.0f));
  EXPECT_EQ(Color::FromString("#0F08"), Color(0.0f, 1.0f, 0.0f, 0x88 / 255.0f));
  EXPECT_EQ(Color::FromString("#0000ff"), Color(0.0f, 0.0f, 1.0f, 1.0f));
  EXPECT_EQ(Color::FromString("#FF000080"),
            Color(1.0f, 0.0f, 0.0f, 0x80 / 255.0f));
  EXPECT_EQ(Color::FromString("  #AaBbCc  "), Color::Create(0xAABBCCFF));
}

TEST(ColorTest, RgbAndRgbaVariants) {
  EXPECT_EQ(Color::FromString("rgb(255, 0, 128)"),
            Color(1.0f, 0.0f, 128.0f / 255.0f, 1.0f));
  EXPECT_EQ(Color::FromString("RGBA( 255 , 0 , 0 , 0.5 )"),
            Color(1.0f, 0.0f, 0.0f, 0.5f));
  EXPECT_EQ(Color::FromString("rgb(100%, 0%, 50%)"),
            Color(1.0f, 0.0f, 0.5f, 1.0f));
  EXPECT_EQ(Color::FromString("rgb(255 0 0 / 50%)"),
            Color(1.0f, 0.0f, 0.0f, 0.5f));
  EXPECT_EQ(Color::FromString("rgb(100% 50% 0% / 0.25)"),
            Color(1.0f, 0.5f, 0.0f, 0.25f));
  // Clamping out-of-range values
  EXPECT_EQ(Color::FromString("rgba(300, -20, 0, 1.5)"),
            Color(1.0f, 0.0f, 0.0f, 1.0f));
}

TEST(ColorTest, HslAndHslaVariants) {
  // Non-multiple-of-30 hue (45deg) tests floating-point HSL conversion
  EXPECT_EQ(Color::FromString("hsl(45, 100%, 50%)"),
            Color(1.0f, 0.75f, 0.0f, 1.0f));
  EXPECT_EQ(Color::FromString("hsl(-120, 100%, 50%)"),
            Color(0.0f, 0.0f, 1.0f, 1.0f));
  EXPECT_EQ(Color::FromString("hsla(120deg, 100%, 50%, 50%)"),
            Color(0.0f, 1.0f, 0.0f, 0.5f));
  EXPECT_EQ(Color::FromString("hsl(0.5turn 100% 50% / 0.25)"),
            Color(0.0f, 1.0f, 1.0f, 0.25f));
  EXPECT_EQ(Color::FromString("hsl(200grad, 100%, 50%)"),
            Color(0.0f, 1.0f, 1.0f, 1.0f));
}

TEST(ColorTest, NamedColorsCaseInsensitive) {
  EXPECT_EQ(Color::FromString("RebeccaPurple"), Color::Create(0x663399FF));
  EXPECT_EQ(Color::FromString("  CORNFLOWERBLUE "), Color::Create(0x6495EDFF));
  EXPECT_EQ(Color::FromString("transparent"), Color(0.0f, 0.0f, 0.0f, 0.0f));
}

TEST(ColorTest, InvalidColors) {
  EXPECT_FALSE(Color::TryParse("").has_value());
  EXPECT_FALSE(Color::TryParse("#12").has_value());
  EXPECT_FALSE(Color::TryParse("#12345").has_value());
  EXPECT_FALSE(Color::TryParse("#1234567").has_value());
  EXPECT_FALSE(Color::TryParse("#gggggg").has_value());
  EXPECT_FALSE(Color::TryParse("rgb(255, 0)").has_value());
  EXPECT_FALSE(Color::TryParse("rgb(255, 0, 50%)").has_value());
  EXPECT_FALSE(Color::TryParse("rgb(255, 0, 0 / 0.5)").has_value());
  EXPECT_FALSE(Color::TryParse("hsl(120, 50, 50%)").has_value());
  EXPECT_FALSE(Color::TryParse("unknowncolor").has_value());
  // Fallback for FromString is opaque black
  EXPECT_EQ(Color::FromString("unknowncolor"), Color(0.0f, 0.0f, 0.0f, 1.0f));
}

}  // namespace
