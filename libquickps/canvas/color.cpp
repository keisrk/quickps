#include <algorithm>
#include <array>
#include <charconv>
#include <cmath>
#include <cstdint>
#include <libquickps/canvas/color.hpp>
#include <numbers>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

#define EPSILON 0.00197f

namespace quickps {
namespace canvas {

namespace {

constexpr bool IsCssSpace(char c) noexcept {
  return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f';
}

constexpr char ToLowerAscii(char c) noexcept {
  return (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c;
}

constexpr std::string_view TrimCssSpaces(std::string_view sv) noexcept {
  while (!sv.empty() && IsCssSpace(sv.front())) {
    sv.remove_prefix(1);
  }
  while (!sv.empty() && IsCssSpace(sv.back())) {
    sv.remove_suffix(1);
  }
  return sv;
}

constexpr bool EqualsIgnoreCase(std::string_view a,
                                std::string_view b) noexcept {
  if (a.size() != b.size()) {
    return false;
  }
  for (size_t i = 0; i < a.size(); ++i) {
    if (ToLowerAscii(a[i]) != ToLowerAscii(b[i])) {
      return false;
    }
  }
  return true;
}

constexpr int HexDigitValue(char c) noexcept {
  if (c >= '0' && c <= '9') {
    return c - '0';
  }
  if (c >= 'a' && c <= 'f') {
    return c - 'a' + 10;
  }
  if (c >= 'A' && c <= 'F') {
    return c - 'A' + 10;
  }
  return -1;
}

std::optional<Color> ParseHex(std::string_view hex) noexcept {
  const size_t len = hex.size();
  if (len != 3 && len != 4 && len != 6 && len != 8) {
    return std::nullopt;
  }
  uint32_t value = 0;
  for (char c : hex) {
    int d = HexDigitValue(c);
    if (d < 0) {
      return std::nullopt;
    }
    value = (value << 4) | static_cast<uint32_t>(d);
  }

  if (len == 3) {
    uint32_t r = (value >> 8) & 0xFu;
    uint32_t g = (value >> 4) & 0xFu;
    uint32_t b = value & 0xFu;
    return Color::Create((r * 0x11u << 24) | (g * 0x11u << 16) |
                         (b * 0x11u << 8) | 0xFFu);
  }
  if (len == 4) {
    uint32_t r = (value >> 12) & 0xFu;
    uint32_t g = (value >> 8) & 0xFu;
    uint32_t b = (value >> 4) & 0xFu;
    uint32_t a = value & 0xFu;
    return Color::Create((r * 0x11u << 24) | (g * 0x11u << 16) |
                         (b * 0x11u << 8) | (a * 0x11u));
  }
  if (len == 6) {
    return Color::Create((value << 8) | 0xFFu);
  }
  return Color::Create(value);
}

struct ParsedNumber {
  float value;
  std::string_view unit;
};

std::optional<ParsedNumber> ParseNumberAndUnit(
    std::string_view token) noexcept {
  token = TrimCssSpaces(token);
  if (token.empty()) {
    return std::nullopt;
  }

  std::string_view num_view = token;
  if (num_view.front() == '+') {
    num_view.remove_prefix(1);
    if (num_view.empty() || num_view.front() == '-' ||
        num_view.front() == '+') {
      return std::nullopt;
    }
  }

  float val = 0.0f;
  const char *first = num_view.data();
  const char *last = first + num_view.size();
  auto [ptr, ec] = std::from_chars(first, last, val);
  if (ec != std::errc{} || ptr == first || !std::isfinite(val)) {
    return std::nullopt;
  }

  std::string_view unit(ptr, static_cast<size_t>(last - ptr));
  for (char c : unit) {
    if (IsCssSpace(c)) {
      return std::nullopt;
    }
  }
  return ParsedNumber{val, unit};
}

struct ArgList {
  std::array<std::string_view, 4> tokens{};
  size_t count = 0;
};

std::optional<ArgList> SplitFunctionArgs(std::string_view body) noexcept {
  body = TrimCssSpaces(body);
  if (body.empty()) {
    return std::nullopt;
  }

  ArgList out;
  if (body.find(',') != std::string_view::npos) {
    // Legacy comma-separated syntax: c0, c1, c2 [, a]
    if (body.find('/') != std::string_view::npos) {
      return std::nullopt;
    }
    while (true) {
      if (out.count >= 4) {
        return std::nullopt;
      }
      size_t comma = body.find(',');
      if (comma == std::string_view::npos) {
        auto tok = TrimCssSpaces(body);
        if (tok.empty()) {
          return std::nullopt;
        }
        out.tokens[out.count++] = tok;
        break;
      }
      auto tok = TrimCssSpaces(body.substr(0, comma));
      if (tok.empty()) {
        return std::nullopt;
      }
      out.tokens[out.count++] = tok;
      body.remove_prefix(comma + 1);
    }
  } else {
    // CSS Color 4 space-separated syntax: c0 c1 c2 [/ a]
    std::optional<std::string_view> alpha_part;
    size_t slash = body.find('/');
    if (slash != std::string_view::npos) {
      auto rest = body.substr(slash + 1);
      if (rest.find('/') != std::string_view::npos) {
        return std::nullopt;
      }
      alpha_part = TrimCssSpaces(rest);
      if (alpha_part->empty()) {
        return std::nullopt;
      }
      body = TrimCssSpaces(body.substr(0, slash));
    }

    while (!body.empty()) {
      if (out.count >= 3) {
        return std::nullopt;
      }
      size_t i = 0;
      while (i < body.size() && !IsCssSpace(body[i])) {
        ++i;
      }
      out.tokens[out.count++] = body.substr(0, i);
      body = TrimCssSpaces(body.substr(i));
    }

    if (out.count != 3) {
      return std::nullopt;
    }
    if (alpha_part.has_value()) {
      out.tokens[out.count++] = *alpha_part;
    }
  }

  if (out.count != 3 && out.count != 4) {
    return std::nullopt;
  }
  return out;
}

std::optional<float> ParseAlpha(std::string_view token) noexcept {
  auto parsed = ParseNumberAndUnit(token);
  if (!parsed.has_value()) {
    return std::nullopt;
  }
  if (parsed->unit.empty()) {
    return std::clamp(parsed->value, 0.0f, 1.0f);
  }
  if (parsed->unit == "%") {
    return std::clamp(parsed->value / 100.0f, 0.0f, 1.0f);
  }
  return std::nullopt;
}

std::optional<Color> ParseRgbArgs(std::string_view body) noexcept {
  auto args = SplitFunctionArgs(body);
  if (!args.has_value()) {
    return std::nullopt;
  }

  std::array<ParsedNumber, 3> comps{};
  for (size_t i = 0; i < 3; ++i) {
    auto p = ParseNumberAndUnit(args->tokens[i]);
    if (!p.has_value()) {
      return std::nullopt;
    }
    comps[i] = *p;
  }

  const bool is_percent = (comps[0].unit == "%");
  std::array<float, 3> rgb{};
  for (size_t i = 0; i < 3; ++i) {
    if (is_percent) {
      if (comps[i].unit != "%") {
        return std::nullopt;
      }
      rgb[i] = std::clamp(comps[i].value / 100.0f, 0.0f, 1.0f);
    } else {
      if (!comps[i].unit.empty()) {
        return std::nullopt;
      }
      rgb[i] = std::clamp(comps[i].value / 255.0f, 0.0f, 1.0f);
    }
  }

  float alpha = 1.0f;
  if (args->count == 4) {
    auto a = ParseAlpha(args->tokens[3]);
    if (!a.has_value()) {
      return std::nullopt;
    }
    alpha = *a;
  }

  return Color(rgb[0], rgb[1], rgb[2], alpha);
}

std::optional<float> ParseHueDegrees(std::string_view token) noexcept {
  auto parsed = ParseNumberAndUnit(token);
  if (!parsed.has_value()) {
    return std::nullopt;
  }
  if (parsed->unit.empty() || EqualsIgnoreCase(parsed->unit, "deg")) {
    return parsed->value;
  }
  if (EqualsIgnoreCase(parsed->unit, "grad")) {
    return parsed->value * 0.9f;
  }
  if (EqualsIgnoreCase(parsed->unit, "rad")) {
    return parsed->value * (180.0f / std::numbers::pi_v<float>);
  }
  if (EqualsIgnoreCase(parsed->unit, "turn")) {
    return parsed->value * 360.0f;
  }
  return std::nullopt;
}

std::optional<float> ParsePercentage(std::string_view token) noexcept {
  auto parsed = ParseNumberAndUnit(token);
  if (!parsed.has_value() || parsed->unit != "%") {
    return std::nullopt;
  }
  return std::clamp(parsed->value, 0.0f, 100.0f);
}

using NamedColor = std::pair<std::string_view, uint32_t>;

constexpr std::array<NamedColor, 149> kNamedColors = {{
    {"aliceblue", 0xF0F8FFFF},
    {"antiquewhite", 0xFAEBD7FF},
    {"aqua", 0x00FFFFFF},
    {"aquamarine", 0x7FFFD4FF},
    {"azure", 0xF0FFFFFF},
    {"beige", 0xF5F5DCFF},
    {"bisque", 0xFFE4C4FF},
    {"black", 0x000000FF},
    {"blanchedalmond", 0xFFEBCDFF},
    {"blue", 0x0000FFFF},
    {"blueviolet", 0x8A2BE2FF},
    {"brown", 0xA52A2AFF},
    {"burlywood", 0xDEB887FF},
    {"cadetblue", 0x5F9EA0FF},
    {"chartreuse", 0x7FFF00FF},
    {"chocolate", 0xD2691EFF},
    {"coral", 0xFF7F50FF},
    {"cornflowerblue", 0x6495EDFF},
    {"cornsilk", 0xFFF8DCFF},
    {"crimson", 0xDC143CFF},
    {"cyan", 0x00FFFFFF},
    {"darkblue", 0x00008BFF},
    {"darkcyan", 0x008B8BFF},
    {"darkgoldenrod", 0xB8860BFF},
    {"darkgray", 0xA9A9A9FF},
    {"darkgreen", 0x006400FF},
    {"darkgrey", 0xA9A9A9FF},
    {"darkkhaki", 0xBDB76BFF},
    {"darkmagenta", 0x8B008BFF},
    {"darkolivegreen", 0x556B2FFF},
    {"darkorange", 0xFF8C00FF},
    {"darkorchid", 0x9932CCFF},
    {"darkred", 0x8B0000FF},
    {"darksalmon", 0xE9967AFF},
    {"darkseagreen", 0x8FBC8FFF},
    {"darkslateblue", 0x483D8BFF},
    {"darkslategray", 0x2F4F4FFF},
    {"darkslategrey", 0x2F4F4FFF},
    {"darkturquoise", 0x00CED1FF},
    {"darkviolet", 0x9400D3FF},
    {"deeppink", 0xFF1493FF},
    {"deepskyblue", 0x00BFFFFF},
    {"dimgray", 0x696969FF},
    {"dimgrey", 0x696969FF},
    {"dodgerblue", 0x1E90FFFF},
    {"firebrick", 0xB22222FF},
    {"floralwhite", 0xFFFAF0FF},
    {"forestgreen", 0x228B22FF},
    {"fuchsia", 0xFF00FFFF},
    {"gainsboro", 0xDCDCDCFF},
    {"ghostwhite", 0xF8F8FFFF},
    {"gold", 0xFFD700FF},
    {"goldenrod", 0xDAA520FF},
    {"gray", 0x808080FF},
    {"green", 0x008000FF},
    {"greenyellow", 0xADFF2FFF},
    {"grey", 0x808080FF},
    {"honeydew", 0xF0FFF0FF},
    {"hotpink", 0xFF69B4FF},
    {"indianred", 0xCD5C5CFF},
    {"indigo", 0x4B0082FF},
    {"ivory", 0xFFFFF0FF},
    {"khaki", 0xF0E68CFF},
    {"lavender", 0xE6E6FAFF},
    {"lavenderblush", 0xFFF0F5FF},
    {"lawngreen", 0x7CFC00FF},
    {"lemonchiffon", 0xFFFACDFF},
    {"lightblue", 0xADD8E6FF},
    {"lightcoral", 0xF08080FF},
    {"lightcyan", 0xE0FFFFFF},
    {"lightgoldenrodyellow", 0xFAFAD2FF},
    {"lightgray", 0xD3D3D3FF},
    {"lightgreen", 0x90EE90FF},
    {"lightgrey", 0xD3D3D3FF},
    {"lightpink", 0xFFB6C1FF},
    {"lightsalmon", 0xFFA07AFF},
    {"lightseagreen", 0x20B2AAFF},
    {"lightskyblue", 0x87CEFAFF},
    {"lightslategray", 0x778899FF},
    {"lightslategrey", 0x778899FF},
    {"lightsteelblue", 0xB0C4DEFF},
    {"lightyellow", 0xFFFFE0FF},
    {"lime", 0x00FF00FF},
    {"limegreen", 0x32CD32FF},
    {"linen", 0xFAF0E6FF},
    {"magenta", 0xFF00FFFF},
    {"maroon", 0x800000FF},
    {"mediumaquamarine", 0x66CDAAFF},
    {"mediumblue", 0x0000CDFF},
    {"mediumorchid", 0xBA55D3FF},
    {"mediumpurple", 0x9370DBFF},
    {"mediumseagreen", 0x3CB371FF},
    {"mediumslateblue", 0x7B68EEFF},
    {"mediumspringgreen", 0x00FA9AFF},
    {"mediumturquoise", 0x48D1CCFF},
    {"mediumvioletred", 0xC71585FF},
    {"midnightblue", 0x191970FF},
    {"mintcream", 0xF5FFFAFF},
    {"mistyrose", 0xFFE4E1FF},
    {"moccasin", 0xFFE4B5FF},
    {"navajowhite", 0xFFDEADFF},
    {"navy", 0x000080FF},
    {"oldlace", 0xFDF5E6FF},
    {"olive", 0x808000FF},
    {"olivedrab", 0x6B8E23FF},
    {"orange", 0xFFA500FF},
    {"orangered", 0xFF4500FF},
    {"orchid", 0xDA70D6FF},
    {"palegoldenrod", 0xEEE8AAFF},
    {"palegreen", 0x98FB98FF},
    {"paleturquoise", 0xAFEEEEFF},
    {"palevioletred", 0xDB7093FF},
    {"papayawhip", 0xFFEFD5FF},
    {"peachpuff", 0xFFDAB9FF},
    {"peru", 0xCD853FFF},
    {"pink", 0xFFC0CBFF},
    {"plum", 0xDDA0DDFF},
    {"powderblue", 0xB0E0E6FF},
    {"purple", 0x800080FF},
    {"rebeccapurple", 0x663399FF},  // Source: CSS Color Level 4
    {"red", 0xFF0000FF},
    {"rosybrown", 0xBC8F8FFF},
    {"royalblue", 0x4169E1FF},
    {"saddlebrown", 0x8B4513FF},
    {"salmon", 0xFA8072FF},
    {"sandybrown", 0xF4A460FF},
    {"seagreen", 0x2E8B57FF},
    {"seashell", 0xFFF5EEFF},
    {"sienna", 0xA0522DFF},
    {"silver", 0xC0C0C0FF},
    {"skyblue", 0x87CEEBFF},
    {"slateblue", 0x6A5ACDFF},
    {"slategray", 0x708090FF},
    {"slategrey", 0x708090FF},
    {"snow", 0xFFFAFAFF},
    {"springgreen", 0x00FF7FFF},
    {"steelblue", 0x4682B4FF},
    {"tan", 0xD2B48CFF},
    {"teal", 0x008080FF},
    {"thistle", 0xD8BFD8FF},
    {"tomato", 0xFF6347FF},
    {"transparent", 0x00000000},
    {"turquoise", 0x40E0D0FF},
    {"violet", 0xEE82EEFF},
    {"wheat", 0xF5DEB3FF},
    {"white", 0xFFFFFFFF},
    {"whitesmoke", 0xF5F5F5FF},
    {"yellow", 0xFFFF00FF},
    {"yellowgreen", 0x9ACD32FF},
}};

static_assert(std::ranges::is_sorted(kNamedColors, {}, &NamedColor::first));

}  // namespace

bool operator==(const Color &lhs, const Color &rhs) {
  return std::fabs(lhs.r - rhs.r) < EPSILON &&
         std::fabs(lhs.g - rhs.g) < EPSILON &&
         std::fabs(lhs.b - rhs.b) < EPSILON &&
         std::fabs(lhs.a - rhs.a) < EPSILON;
}

bool operator!=(const Color &lhs, const Color &rhs) { return !(lhs == rhs); }

Color Color::Create(uint32_t rgba) {
  float r = static_cast<float>(rgba >> 24) / 255.0f;
  float g = static_cast<float>((rgba >> 16) & 0xff) / 255.0f;
  float b = static_cast<float>((rgba >> 8) & 0xff) / 255.0f;
  float a = static_cast<float>(rgba & 0xff) / 255.0f;
  return Color(r, g, b, a);
}

// https://drafts.csswg.org/css-color-4/#hsl-to-rgb
Color Color::FromHsla(float hue, float saturation, float lightness,
                      float alpha) {
  float h = std::fmod(hue, 360.0f);
  if (h < 0.0f) {
    h += 360.0f;
  }
  const float s = std::clamp(saturation, 0.0f, 100.0f) / 100.0f;
  const float l = std::clamp(lightness, 0.0f, 100.0f) / 100.0f;
  const auto f = [=](float n) {
    const float k = std::fmod(n + h / 30.0f, 12.0f);
    const float a = s * std::min(l, 1.0f - l);
    return l - a * std::max(-1.0f, std::min({k - 3.0f, 9.0f - k, 1.0f}));
  };

  return Color(f(0.0f), f(8.0f), f(4.0f), std::clamp(alpha, 0.0f, 1.0f));
}

std::optional<Color> Color::TryParse(std::string_view str) {
  str = TrimCssSpaces(str);
  if (str.empty()) {
    return std::nullopt;
  }

  if (str.front() == '#') {
    return ParseHex(str.substr(1));
  }

  const size_t open_paren = str.find('(');
  if (open_paren != std::string_view::npos) {
    if (str.back() != ')') {
      return std::nullopt;
    }
    const std::string_view fn = str.substr(0, open_paren);
    const std::string_view body =
        str.substr(open_paren + 1, str.size() - open_paren - 2);

    if (EqualsIgnoreCase(fn, "rgb") || EqualsIgnoreCase(fn, "rgba")) {
      return ParseRgbArgs(body);
    }
    if (EqualsIgnoreCase(fn, "hsl") || EqualsIgnoreCase(fn, "hsla")) {
      auto args = SplitFunctionArgs(body);
      if (!args.has_value()) {
        return std::nullopt;
      }
      auto hue = ParseHueDegrees(args->tokens[0]);
      auto sat = ParsePercentage(args->tokens[1]);
      auto light = ParsePercentage(args->tokens[2]);
      if (!hue.has_value() || !sat.has_value() || !light.has_value()) {
        return std::nullopt;
      }
      float alpha = 1.0f;
      if (args->count == 4) {
        auto a = ParseAlpha(args->tokens[3]);
        if (!a.has_value()) {
          return std::nullopt;
        }
        alpha = *a;
      }
      return Color::FromHsla(*hue, *sat, *light, alpha);
    }
    return std::nullopt;
  }

  return Color::FromName(str);
}

Color Color::FromString(std::string_view str) {
  return TryParse(str).value_or(Color::Create(0x000000FF));
}

std::optional<Color> Color::FromName(std::string_view str) {
  constexpr size_t kMaxNameLen = 20;  // "lightgoldenrodyellow"
  if (str.empty() || str.size() > kMaxNameLen) {
    return std::nullopt;
  }

  std::array<char, kMaxNameLen> lower_buf{};
  for (size_t i = 0; i < str.size(); ++i) {
    lower_buf[i] = ToLowerAscii(str[i]);
  }
  std::string_view key(lower_buf.data(), str.size());

  auto it = std::ranges::lower_bound(kNamedColors, key, {}, &NamedColor::first);
  if (it == kNamedColors.end() || it->first != key) {
    return std::nullopt;
  }
  return Color::Create(it->second);
}

}  // namespace canvas
}  // namespace quickps
