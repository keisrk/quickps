#ifndef QUICKPS_CANVAS_STYLE_H_
#define QUICKPS_CANVAS_STYLE_H_

#include <blend2d/blend2d.h>
#include <libquickps/canvas/color.hpp>
#include <variant>

namespace quickps {
namespace canvas {

struct Style final {
  using Type = std::variant<Color, BLGradient, BLPattern>;
  Type value;

  Style(const Color &color);
  Style(const BLGradient &gradient);
  Style(const BLPattern &pattern);

public:
  static Style FromColor(const Color &);
  static Style FromGradient(const BLGradient &gradient);
  static Style FromPattern(const BLPattern &pattern);
};

} // namespace canvas
} // namespace quickps

#endif // QUICKPS_CANVAS_STYLE_H_
/*

class Style {
public:
    bool isValid() const { return !std::holds_alternative<Invalid>(m_style); }
    bool isCurrentColor() const { return
std::holds_alternative<CurrentColor>(m_style); } std::optional<float>
overrideAlpha() const { return std::get<CurrentColor>(m_style).overrideAlpha; }

    String color() const;
    std::optional<Color> srgbaColor() const;
    RefPtr<CanvasGradient> canvasGradient() const;
    RefPtr<CanvasPattern> canvasPattern() const;

    void applyFillColor(GraphicsContext&) const;
    void applyStrokeColor(GraphicsContext&) const;

    bool isEquivalentColor(const CanvasStyle&) const;
    bool isEquivalent(const SRGBA<float>&) const;

private:
    struct Invalid { };

    struct CurrentColor {
        std::optional<float> overrideAlpha;
    };

    CanvasStyle(CurrentColor);

    std::variant<Invalid, Color, RefPtr<CanvasGradient>, RefPtr<CanvasPattern>,
CurrentColor> m_style;
};
*/
