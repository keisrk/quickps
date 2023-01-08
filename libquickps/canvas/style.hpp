#ifndef QUICKPS_CANVAS_STYLE_H_
#define QUICKPS_CANVAS_STYLE_H_

#include <libquickps/canvas/color.hpp>

namespace quickps {
namespace canvas {

class Style {
public:
  Style FromColor(const Color &);
  Style FromPattern();
};

} // namespace canvas
} // namespace quickps

#endif // QUICKPS_CANVAS_STYLE_H_
