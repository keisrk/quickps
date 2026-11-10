#ifndef QUICKPS_CANVAS_FONT_H_
#define QUICKPS_CANVAS_FONT_H_

#include <blend2d/blend2d.h>

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace quickps {
namespace canvas {

struct ParsedFontSpec {
  BLFontStyle style = BL_FONT_STYLE_NORMAL;
  uint32_t weight = BL_FONT_WEIGHT_NORMAL;
  uint32_t stretch = BL_FONT_STRETCH_NORMAL;
  bool small_caps = false;
  float size_px = 10.0f;
  std::vector<std::string> families;

  bool operator==(const ParsedFontSpec &other) const = default;
};

std::optional<ParsedFontSpec> TryParseCssFont(std::string_view input);

std::optional<double> TryParseCssLetterSpacing(std::string_view input,
                                               double font_size_px);

class FontRegistry final {
 public:
  static FontRegistry &GetInstance();

  bool RegisterFontFile(const std::filesystem::path &path);
  bool RegisterFontMemory(const void *data, size_t size);
  BLFont ResolveFont(const ParsedFontSpec &spec);

  size_t LoadedFaceCount() const noexcept;
  size_t LoadedFamilyCount() const noexcept;

 private:
  FontRegistry();

  void EnsureFallbackLoaded();
  void EnsureDirectoriesScanned();
  bool QuerySingleFamily(std::string_view family,
                         const BLFontQueryProperties &props, BLFontFace &out);

  BLFontManager manager_;
  BLFontFace fallback_face_;
  std::vector<std::filesystem::path> unindexed_files_;
  bool dirs_scanned_ = false;
  bool all_files_indexed_ = false;
};

}  // namespace canvas
}  // namespace quickps

#endif  // QUICKPS_CANVAS_FONT_H_
