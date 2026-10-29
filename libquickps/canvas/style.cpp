#include <libquickps/canvas/style.hpp>

namespace quickps {
namespace canvas {

Style Style::FromColor(const Color &color) { return Style(color); }

Style Style::FromGradient(const BLGradient &gradient) {
  return Style(gradient);
}

Style Style::FromPattern(const BLPattern &pattern) { return Style(pattern); }

Style::Style(const Color &color) : value(color) {}

Style::Style(const BLGradient &gradient) : value(gradient) {}

Style::Style(const BLPattern &pattern) : value(pattern) {}

} // namespace canvas
} // namespace quickps
