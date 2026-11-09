#ifndef QUICKPS_CANVAS_COLOR_H_
#define QUICKPS_CANVAS_COLOR_H_

#include <cstdint>
#include <optional>
#include <string_view>

namespace quickps {
namespace canvas {

struct Color final {
  float r, g, b, a;
  static Color Create(uint32_t rgba);
  static std::optional<Color> TryParse(std::string_view str);
  static Color FromString(std::string_view str);
  constexpr Color(float r, float g, float b, float a)
      : r(r), g(g), b(b), a(a) {}

 private:
  static Color FromHsla(float hue, float saturation, float lightness,
                        float alpha);
  static std::optional<Color> FromName(std::string_view str);
};

bool operator==(const Color &, const Color &);
bool operator!=(const Color &, const Color &);

}  // namespace canvas
}  // namespace quickps

#endif  // QUICKPS_CANVAS_COLOR_H_
