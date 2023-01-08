#ifndef QUICKPS_CANVAS_COLOR_H_
#define QUICKPS_CANVAS_COLOR_H_

#include <string>

namespace quickps {
namespace canvas {

struct Color {
  double r, g, b, a;
  static Color Create(uint32_t rgba);
  static Color FromString(const std::string &str);

private:
  static Color FromHsla(int, int, int, double);
  static Color FromStringName(const std::string &str);
};

bool operator==(const Color &, const Color &);
bool operator!=(const Color &, const Color &);

} // namespace canvas
} // namespace quickps

#endif // QUICKPS_CANVAS_COLOR_H_
