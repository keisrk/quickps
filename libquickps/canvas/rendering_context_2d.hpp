#ifndef QUICKPS_CANVAS_RENDERING_CONTEXT_2D_H_
#define QUICKPS_CANVAS_RENDERING_CONTEXT_2D_H_

#include <cairomm/cairomm.h>
#include <cairomm/surface.h>
#include <libquickps/quickjsmm/esm.hpp>
#include <memory>

namespace quickps {
namespace canvas {

class Context final {
public:
  Context(Cairo::RefPtr<Cairo::Context> ctx);
  Context(const Context &) = delete;
  Context &operator=(const Context &) = delete;

private:
  Cairo::RefPtr<Cairo::Context> context_;
};

class Canvas final {
public:
  Canvas(int width, int height);
  Canvas(const Canvas &) = delete;
  Canvas &operator=(const Canvas &) = delete;
  const std::unique_ptr<Context> CreateContext();

private:
  Cairo::RefPtr<Cairo::ImageSurface> surface_;
};

} // namespace canvas
} // namespace quickps

#endif // QUICKPS_CANVAS_RENDERING_CONTEXT_2D_H_
