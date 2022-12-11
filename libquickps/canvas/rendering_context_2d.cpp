#include <libquickps/canvas/rendering_context_2d.hpp>

namespace quickps {
namespace canvas {

Context::Context(Cairo::RefPtr<Cairo::Context> ctx) : context_(ctx) {}

Canvas::Canvas(int width, int height)
    : surface_(Cairo::ImageSurface::create(Cairo::Format::FORMAT_ARGB32, width,
                                           height)) {}

const std::unique_ptr<Context> Canvas::CreateContext() {
  return std::make_unique<Context>(Cairo::Context::create(surface_));
}

} // namespace canvas
} // namespace quickps
