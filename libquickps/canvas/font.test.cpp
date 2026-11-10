#include <gtest/gtest.h>

#include <libquickps/canvas/font.hpp>

namespace {
using namespace quickps::canvas;

TEST(FontParserTest, BasicCssFontShorthands) {
  auto spec = TryParseCssFont("12px sans-serif");
  ASSERT_TRUE(spec.has_value());
  EXPECT_EQ(spec->style, BL_FONT_STYLE_NORMAL);
  EXPECT_EQ(spec->weight, BL_FONT_WEIGHT_NORMAL);
  EXPECT_EQ(spec->stretch, BL_FONT_STRETCH_NORMAL);
  EXPECT_FALSE(spec->small_caps);
  EXPECT_FLOAT_EQ(spec->size_px, 12.0f);
  ASSERT_EQ(spec->families.size(), 1U);
  EXPECT_EQ(spec->families[0], "sans-serif");
}

TEST(FontParserTest, StyleWeightVariantAndQuotedFamilies) {
  auto spec = TryParseCssFont(
      "italic small-caps bold 24px/1.4 \"Helvetica Neue\", 'ABeeZee', "
      "sans-serif");
  ASSERT_TRUE(spec.has_value());
  EXPECT_EQ(spec->style, BL_FONT_STYLE_ITALIC);
  EXPECT_EQ(spec->weight, BL_FONT_WEIGHT_BOLD);
  EXPECT_TRUE(spec->small_caps);
  EXPECT_FLOAT_EQ(spec->size_px, 24.0f);
  ASSERT_EQ(spec->families.size(), 3U);
  EXPECT_EQ(spec->families[0], "Helvetica Neue");
  EXPECT_EQ(spec->families[1], "ABeeZee");
  EXPECT_EQ(spec->families[2], "sans-serif");
}

TEST(FontParserTest, NumericWeightObliqueStretchAndPtUnits) {
  auto spec = TryParseCssFont("oblique 600 condensed 18pt Times New Roman");
  ASSERT_TRUE(spec.has_value());
  EXPECT_EQ(spec->style, BL_FONT_STYLE_OBLIQUE);
  EXPECT_EQ(spec->weight, 600U);
  EXPECT_EQ(spec->stretch, BL_FONT_STRETCH_CONDENSED);
  EXPECT_FLOAT_EQ(spec->size_px, 24.0f);
  ASSERT_EQ(spec->families.size(), 1U);
  EXPECT_EQ(spec->families[0], "Times New Roman");
}

TEST(FontParserTest, RejectsInvalidCssFonts) {
  EXPECT_FALSE(TryParseCssFont("").has_value());
  EXPECT_FALSE(TryParseCssFont("sans-serif").has_value());
  EXPECT_FALSE(TryParseCssFont("12px").has_value());
  EXPECT_FALSE(TryParseCssFont("-12px sans-serif").has_value());
  EXPECT_FALSE(TryParseCssFont("0px sans-serif").has_value());
  EXPECT_FALSE(TryParseCssFont("12px \"unclosed").has_value());
  EXPECT_FALSE(TryParseCssFont("12px inherit").has_value());
  EXPECT_FALSE(TryParseCssFont("unknown-style 12px sans-serif").has_value());
}

TEST(FontParserTest, LetterSpacingUnits) {
  EXPECT_DOUBLE_EQ(TryParseCssLetterSpacing("normal", 20.0).value(), 0.0);
  EXPECT_DOUBLE_EQ(TryParseCssLetterSpacing("4px", 20.0).value(), 4.0);
  EXPECT_DOUBLE_EQ(TryParseCssLetterSpacing("0.1em", 20.0).value(), 2.0);
  EXPECT_DOUBLE_EQ(TryParseCssLetterSpacing("-0.05em", 20.0).value(), -1.0);
  EXPECT_FALSE(TryParseCssLetterSpacing("invalid", 20.0).has_value());
}

TEST(FontRegistryTest, ResolvesByFamilyNameAndFallback) {
  auto &registry = FontRegistry::GetInstance();

  auto abeezee_spec = TryParseCssFont("32px ABeeZee");
  ASSERT_TRUE(abeezee_spec.has_value());
  BLFont font = registry.ResolveFont(*abeezee_spec);
  ASSERT_TRUE(font.is_valid());
  EXPECT_FLOAT_EQ(font.size(), 32.0f);
  EXPECT_STREQ(font.face().family_name().data(), "ABeeZee");

  auto fallback_spec =
      TryParseCssFont("20px NonExistentFontFamily, sans-serif");
  ASSERT_TRUE(fallback_spec.has_value());
  BLFont fallback_font = registry.ResolveFont(*fallback_spec);
  ASSERT_TRUE(fallback_font.is_valid());
  EXPECT_FLOAT_EQ(fallback_font.size(), 20.0f);
}

}  // namespace
