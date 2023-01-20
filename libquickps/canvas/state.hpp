#ifndef QUICKPS_CANVAS_STATE_H_
#define QUICKPS_CANVAS_STATE_H_

#include <libquickps/canvas/style.hpp>
#include <string>
#include <vector>

namespace quickps {
namespace canvas {

struct State final {
  Style stroke_style;
  Style fill_style;
  int line_width;
  int line_cap;
  int line_join;
  std::vector<double> line_dash;
  float line_dash_offset;
  float shadow_offset_x;
  float shadow_offset_y;
  float shadow_blur;
  std::string shadow_color;
  double global_alpha;
  std::string global_composite_operation;
  std::string font;
};

} // namespace canvas
} // namespace quickps

#endif // QUICKPS_CANVAS_STATE_H_
