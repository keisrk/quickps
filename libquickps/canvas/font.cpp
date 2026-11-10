#include <algorithm>
#include <array>
#include <charconv>
#include <cmath>
#include <cstdlib>
#include <libquickps/canvas/font.hpp>

namespace quickps {
namespace canvas {

namespace {

constexpr uint8_t kEmbeddedFallbackTtf[] = {
#embed "ABeeZee-Regular.ttf"
};

constexpr bool IsCssSpace(char c) noexcept {
  return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f';
}

constexpr char ToLowerAscii(char c) noexcept {
  return (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c;
}

constexpr bool IsAsciiAlphaNum(char c) noexcept {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
         (c >= '0' && c <= '9');
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

std::string NormalizeStem(std::string_view sv) {
  std::string out;
  out.reserve(sv.size());
  for (char c : sv) {
    if (IsAsciiAlphaNum(c)) {
      out.push_back(ToLowerAscii(c));
    }
  }
  return out;
}

bool IsFontExtension(std::string_view ext) noexcept {
  return EqualsIgnoreCase(ext, ".ttf") || EqualsIgnoreCase(ext, ".otf") ||
         EqualsIgnoreCase(ext, ".ttc");
}

bool IsCssWideKeyword(std::string_view name) noexcept {
  return EqualsIgnoreCase(name, "inherit") ||
         EqualsIgnoreCase(name, "initial") || EqualsIgnoreCase(name, "unset") ||
         EqualsIgnoreCase(name, "revert") || EqualsIgnoreCase(name, "default");
}

std::optional<uint32_t> TryParseFontStretch(std::string_view token) noexcept {
  if (EqualsIgnoreCase(token, "ultra-condensed")) {
    return BL_FONT_STRETCH_ULTRA_CONDENSED;
  }
  if (EqualsIgnoreCase(token, "extra-condensed")) {
    return BL_FONT_STRETCH_EXTRA_CONDENSED;
  }
  if (EqualsIgnoreCase(token, "condensed")) {
    return BL_FONT_STRETCH_CONDENSED;
  }
  if (EqualsIgnoreCase(token, "semi-condensed")) {
    return BL_FONT_STRETCH_SEMI_CONDENSED;
  }
  if (EqualsIgnoreCase(token, "semi-expanded")) {
    return BL_FONT_STRETCH_SEMI_EXPANDED;
  }
  if (EqualsIgnoreCase(token, "expanded")) {
    return BL_FONT_STRETCH_EXPANDED;
  }
  if (EqualsIgnoreCase(token, "extra-expanded")) {
    return BL_FONT_STRETCH_EXTRA_EXPANDED;
  }
  if (EqualsIgnoreCase(token, "ultra-expanded")) {
    return BL_FONT_STRETCH_ULTRA_EXPANDED;
  }
  return std::nullopt;
}

std::optional<uint32_t> TryParseFontWeight(std::string_view token) noexcept {
  if (EqualsIgnoreCase(token, "bold") || EqualsIgnoreCase(token, "bolder")) {
    return BL_FONT_WEIGHT_BOLD;
  }
  if (EqualsIgnoreCase(token, "lighter")) {
    return BL_FONT_WEIGHT_LIGHT;
  }
  uint32_t weight = 0;
  const char *first = token.data();
  const char *last = first + token.size();
  auto [ptr, ec] = std::from_chars(first, last, weight);
  if (ec == std::errc{} && ptr == last && weight >= 1 && weight <= 1000) {
    return weight;
  }
  return std::nullopt;
}

std::optional<float> TryParseFontSizeToken(std::string_view token) noexcept {
  if (EqualsIgnoreCase(token, "xx-small")) return 9.0f;
  if (EqualsIgnoreCase(token, "x-small")) return 10.0f;
  if (EqualsIgnoreCase(token, "small")) return 13.0f;
  if (EqualsIgnoreCase(token, "medium")) return 16.0f;
  if (EqualsIgnoreCase(token, "large")) return 18.0f;
  if (EqualsIgnoreCase(token, "x-large")) return 24.0f;
  if (EqualsIgnoreCase(token, "xx-large")) return 32.0f;
  if (EqualsIgnoreCase(token, "xxx-large")) return 48.0f;
  if (EqualsIgnoreCase(token, "smaller")) return 13.0f;
  if (EqualsIgnoreCase(token, "larger")) return 19.0f;

  if (token.empty()) {
    return std::nullopt;
  }
  std::string_view num_view = token;
  if (num_view.front() == '+') {
    num_view.remove_prefix(1);
    if (num_view.empty() || num_view.front() == '+' ||
        num_view.front() == '-') {
      return std::nullopt;
    }
  }

  float val = 0.0f;
  const char *first = num_view.data();
  const char *last = first + num_view.size();
  auto [ptr, ec] = std::from_chars(first, last, val);
  if (ec != std::errc{} || ptr == first || !std::isfinite(val) || val < 0.0f) {
    return std::nullopt;
  }

  std::string_view unit(ptr, static_cast<size_t>(last - ptr));
  if (unit.empty()) {
    return std::nullopt;
  }

  float factor = 0.0f;
  if (EqualsIgnoreCase(unit, "px")) {
    factor = 1.0f;
  } else if (EqualsIgnoreCase(unit, "pt")) {
    factor = 96.0f / 72.0f;
  } else if (EqualsIgnoreCase(unit, "pc")) {
    factor = 16.0f;
  } else if (EqualsIgnoreCase(unit, "in")) {
    factor = 96.0f;
  } else if (EqualsIgnoreCase(unit, "cm")) {
    factor = 96.0f / 2.54f;
  } else if (EqualsIgnoreCase(unit, "mm")) {
    factor = 96.0f / 25.4f;
  } else if (EqualsIgnoreCase(unit, "q")) {
    factor = 96.0f / 101.6f;
  } else if (EqualsIgnoreCase(unit, "em") || EqualsIgnoreCase(unit, "rem")) {
    factor = 16.0f;
  } else if (EqualsIgnoreCase(unit, "ex") || EqualsIgnoreCase(unit, "ch")) {
    factor = 8.0f;
  } else if (unit == "%") {
    factor = 0.16f;
  } else {
    return std::nullopt;
  }

  float px = val * factor;
  if (!std::isfinite(px) || px <= 0.0f) {
    return std::nullopt;
  }
  return px;
}

std::string_view NextSpaceDelimitedToken(std::string_view &rest) noexcept {
  rest = TrimCssSpaces(rest);
  size_t i = 0;
  while (i < rest.size() && !IsCssSpace(rest[i])) {
    ++i;
  }
  std::string_view tok = rest.substr(0, i);
  rest.remove_prefix(i);
  return tok;
}

std::optional<std::vector<std::string>> ParseFontFamilyList(
    std::string_view input) {
  input = TrimCssSpaces(input);
  if (input.empty()) {
    return std::nullopt;
  }

  std::vector<std::string> families;
  size_t pos = 0;
  const size_t n = input.size();

  while (pos < n) {
    while (pos < n && IsCssSpace(input[pos])) {
      ++pos;
    }
    if (pos >= n || input[pos] == ',') {
      return std::nullopt;
    }

    if (input[pos] == '"' || input[pos] == '\'') {
      const char quote = input[pos++];
      std::string name;
      bool closed = false;
      while (pos < n) {
        char c = input[pos++];
        if (c == '\\') {
          if (pos >= n) {
            return std::nullopt;
          }
          name.push_back(input[pos++]);
        } else if (c == quote) {
          closed = true;
          break;
        } else {
          name.push_back(c);
        }
      }
      if (!closed) {
        return std::nullopt;
      }
      auto trimmed = TrimCssSpaces(name);
      if (trimmed.empty()) {
        return std::nullopt;
      }
      families.emplace_back(trimmed);

      while (pos < n && IsCssSpace(input[pos])) {
        ++pos;
      }
      if (pos < n) {
        if (input[pos] != ',') {
          return std::nullopt;
        }
        ++pos;
        if (pos >= n) {
          return std::nullopt;
        }
      }
    } else {
      size_t start = pos;
      while (pos < n && input[pos] != ',') {
        if (input[pos] == '"' || input[pos] == '\'' || input[pos] == ';') {
          return std::nullopt;
        }
        ++pos;
      }
      std::string_view raw = TrimCssSpaces(input.substr(start, pos - start));
      if (raw.empty() || IsCssWideKeyword(raw)) {
        return std::nullopt;
      }
      std::string collapsed;
      collapsed.reserve(raw.size());
      bool prev_space = false;
      for (char c : raw) {
        if (IsCssSpace(c)) {
          if (!prev_space) {
            collapsed.push_back(' ');
            prev_space = true;
          }
        } else {
          collapsed.push_back(c);
          prev_space = false;
        }
      }
      families.push_back(std::move(collapsed));

      if (pos < n && input[pos] == ',') {
        ++pos;
        if (pos >= n) {
          return std::nullopt;
        }
      }
    }
  }

  if (families.empty()) {
    return std::nullopt;
  }
  return families;
}

}  // namespace

std::optional<ParsedFontSpec> TryParseCssFont(std::string_view input) {
  std::string_view rest = TrimCssSpaces(input);
  if (rest.empty()) {
    return std::nullopt;
  }

  ParsedFontSpec spec;
  bool seen_style = false;
  bool seen_variant = false;
  bool seen_weight = false;
  bool seen_stretch = false;
  bool seen_size = false;

  while (!rest.empty()) {
    std::string_view before_token = rest;
    std::string_view tok = NextSpaceDelimitedToken(rest);
    if (tok.empty()) {
      break;
    }

    // Check if this token is <font-size> or <font-size>/<line-height>
    std::string_view size_part = tok;
    std::string_view slash_part;
    if (size_t slash = tok.find('/'); slash != std::string_view::npos) {
      size_part = tok.substr(0, slash);
      slash_part = tok.substr(slash);
    }

    if (auto size_px = TryParseFontSizeToken(size_part); size_px.has_value()) {
      spec.size_px = *size_px;
      seen_size = true;

      if (!slash_part.empty()) {
        // Token had a '/' attached: e.g. "16px/1.2" or "16px/ 1.2"
        if (slash_part.size() == 1) {
          // "16px/ 1.2" -> consume line-height token
          std::string_view lh = NextSpaceDelimitedToken(rest);
          if (lh.empty()) {
            return std::nullopt;
          }
        }
      } else {
        // Check if next token starts with '/' (e.g. "16px / 1.2" or "16px
        // /1.2")
        std::string_view trimmed_rest = TrimCssSpaces(rest);
        if (!trimmed_rest.empty() && trimmed_rest.front() == '/') {
          std::string_view lh_tok = NextSpaceDelimitedToken(rest);
          if (lh_tok.size() == 1) {
            std::string_view lh = NextSpaceDelimitedToken(rest);
            if (lh.empty()) {
              return std::nullopt;
            }
          }
        }
      }
      break;
    }

    // Otherwise, must be one of [font-style || font-variant || font-weight ||
    // font-stretch]
    if (EqualsIgnoreCase(tok, "normal")) {
      continue;
    }
    if (!seen_style && EqualsIgnoreCase(tok, "italic")) {
      spec.style = BL_FONT_STYLE_ITALIC;
      seen_style = true;
      continue;
    }
    if (!seen_style && EqualsIgnoreCase(tok, "oblique")) {
      spec.style = BL_FONT_STYLE_OBLIQUE;
      seen_style = true;
      continue;
    }
    if (!seen_variant && EqualsIgnoreCase(tok, "small-caps")) {
      spec.small_caps = true;
      seen_variant = true;
      continue;
    }
    if (!seen_weight) {
      if (auto w = TryParseFontWeight(tok); w.has_value()) {
        spec.weight = *w;
        seen_weight = true;
        continue;
      }
    }
    if (!seen_stretch) {
      if (auto s = TryParseFontStretch(tok); s.has_value()) {
        spec.stretch = *s;
        seen_stretch = true;
        continue;
      }
    }

    // Unrecognized prefix token before font-size
    (void)before_token;
    return std::nullopt;
  }

  if (!seen_size) {
    return std::nullopt;
  }

  auto families = ParseFontFamilyList(rest);
  if (!families.has_value()) {
    return std::nullopt;
  }
  spec.families = std::move(*families);
  return spec;
}

std::optional<double> TryParseCssLetterSpacing(std::string_view input,
                                               double font_size_px) {
  input = TrimCssSpaces(input);
  if (input.empty()) {
    return std::nullopt;
  }
  if (EqualsIgnoreCase(input, "normal")) {
    return 0.0;
  }

  std::string_view num_view = input;
  if (num_view.front() == '+') {
    num_view.remove_prefix(1);
    if (num_view.empty() || num_view.front() == '+' ||
        num_view.front() == '-') {
      return std::nullopt;
    }
  }

  double val = 0.0;
  const char *first = num_view.data();
  const char *last = first + num_view.size();
  auto [ptr, ec] = std::from_chars(first, last, val);
  if (ec != std::errc{} || ptr == first || !std::isfinite(val)) {
    return std::nullopt;
  }

  std::string_view unit(ptr, static_cast<size_t>(last - ptr));
  if (EqualsIgnoreCase(unit, "px")) {
    return val;
  }
  if (EqualsIgnoreCase(unit, "em") || EqualsIgnoreCase(unit, "rem")) {
    return val * font_size_px;
  }
  if (EqualsIgnoreCase(unit, "pt")) {
    return val * (96.0 / 72.0);
  }
  return std::nullopt;
}

FontRegistry &FontRegistry::GetInstance() {
  static FontRegistry kInstance;
  return kInstance;
}

FontRegistry::FontRegistry() { manager_.create(); }

void FontRegistry::EnsureFallbackLoaded() {
  if (fallback_face_.is_valid()) {
    return;
  }
  BLFontData font_data;
  if (font_data.create_from_data(kEmbeddedFallbackTtf,
                                 sizeof(kEmbeddedFallbackTtf)) == BL_SUCCESS) {
    if (fallback_face_.create_from_data(font_data, 0) == BL_SUCCESS) {
      manager_.add_face(fallback_face_);
    }
  }
}

void FontRegistry::EnsureDirectoriesScanned() {
  if (dirs_scanned_) {
    return;
  }
  dirs_scanned_ = true;

  std::vector<std::filesystem::path> roots = {
      "/usr/share/fonts",
      "/usr/local/share/fonts",
  };
  if (const char *home = std::getenv("HOME"); home && *home) {
    std::filesystem::path home_path(home);
    roots.push_back(home_path / ".local/share/fonts");
    roots.push_back(home_path / ".fonts");
  }
  if (const char *env_path = std::getenv("QUICKPS_FONT_PATH");
      env_path && *env_path) {
    std::string_view sv(env_path);
    while (!sv.empty()) {
      size_t colon = sv.find(':');
      std::string_view part =
          (colon == std::string_view::npos) ? sv : sv.substr(0, colon);
      if (!part.empty()) {
        roots.emplace_back(part);
      }
      if (colon == std::string_view::npos) {
        break;
      }
      sv.remove_prefix(colon + 1);
    }
  }

  std::error_code ec;
  for (const auto &root : roots) {
    if (!std::filesystem::exists(root, ec)) {
      continue;
    }
    if (std::filesystem::is_regular_file(root, ec)) {
      if (IsFontExtension(root.extension().string())) {
        unindexed_files_.push_back(root);
      }
      continue;
    }
    for (auto it = std::filesystem::recursive_directory_iterator(
             root, std::filesystem::directory_options::skip_permission_denied,
             ec);
         !ec && it != std::filesystem::recursive_directory_iterator();
         it.increment(ec)) {
      if (it->is_regular_file(ec)) {
        const auto &p = it->path();
        if (IsFontExtension(p.extension().string())) {
          unindexed_files_.push_back(p);
        }
      }
    }
  }
}

bool FontRegistry::RegisterFontFile(const std::filesystem::path &path) {
  BLFontData font_data;
  constexpr auto kReadFlags = static_cast<BLFileReadFlags>(
      BL_FILE_READ_MMAP_ENABLED | BL_FILE_READ_MMAP_AVOID_SMALL);
  if (font_data.create_from_file(path.c_str(), kReadFlags) != BL_SUCCESS) {
    return false;
  }

  bool added_any = false;
  const uint32_t count = font_data.face_count();
  for (uint32_t i = 0; i < count; ++i) {
    BLFontFace face;
    if (face.create_from_data(font_data, i) == BL_SUCCESS) {
      if (manager_.add_face(face) == BL_SUCCESS) {
        added_any = true;
      }
    }
  }
  return added_any;
}

bool FontRegistry::RegisterFontMemory(const void *data, size_t size) {
  BLFontData font_data;
  if (font_data.create_from_data(data, size) != BL_SUCCESS) {
    return false;
  }

  bool added_any = false;
  const uint32_t count = font_data.face_count();
  for (uint32_t i = 0; i < count; ++i) {
    BLFontFace face;
    if (face.create_from_data(font_data, i) == BL_SUCCESS) {
      if (manager_.add_face(face) == BL_SUCCESS) {
        added_any = true;
      }
    }
  }
  return added_any;
}

bool FontRegistry::QuerySingleFamily(std::string_view family,
                                     const BLFontQueryProperties &props,
                                     BLFontFace &out) {
  BLStringView name_view{family.data(), family.size()};
  if (manager_.query_face(name_view, props, out) == BL_SUCCESS) {
    return true;
  }

  // Support direct path to a .ttf/.otf/.ttc file as family name
  if (family.find('/') != std::string_view::npos ||
      IsFontExtension(family.size() >= 4 ? family.substr(family.size() - 4)
                                         : "")) {
    std::error_code ec;
    std::filesystem::path p(family);
    if (std::filesystem::is_regular_file(p, ec)) {
      BLFontFace direct_face;
      constexpr auto kReadFlags = static_cast<BLFileReadFlags>(
          BL_FILE_READ_MMAP_ENABLED | BL_FILE_READ_MMAP_AVOID_SMALL);
      if (direct_face.create_from_file(p.c_str(), kReadFlags) == BL_SUCCESS) {
        manager_.add_face(direct_face);
        out = std::move(direct_face);
        return true;
      }
    }
  }

  EnsureDirectoriesScanned();

  // Tier 2A: Load only files whose normalized stem starts with the normalized
  // family name.
  const std::string target_stem = NormalizeStem(family);
  if (!target_stem.empty() && !unindexed_files_.empty()) {
    bool loaded_candidate = false;
    auto it = unindexed_files_.begin();
    while (it != unindexed_files_.end()) {
      std::string file_stem = NormalizeStem(it->stem().string());
      if (file_stem.starts_with(target_stem) ||
          target_stem.starts_with(file_stem)) {
        loaded_candidate |= RegisterFontFile(*it);
        it = unindexed_files_.erase(it);
      } else {
        ++it;
      }
    }
    if (loaded_candidate &&
        manager_.query_face(name_view, props, out) == BL_SUCCESS) {
      return true;
    }
  }

  // Tier 2B: Full scan of any remaining unindexed font files (once).
  if (!all_files_indexed_ && !unindexed_files_.empty()) {
    for (const auto &path : unindexed_files_) {
      RegisterFontFile(path);
    }
    unindexed_files_.clear();
    all_files_indexed_ = true;
    if (manager_.query_face(name_view, props, out) == BL_SUCCESS) {
      return true;
    }
  }

  return false;
}

BLFont FontRegistry::ResolveFont(const ParsedFontSpec &spec) {
  EnsureFallbackLoaded();

  const BLFontQueryProperties props{
      .style = static_cast<uint32_t>(spec.style),
      .weight = spec.weight,
      .stretch = spec.stretch,
  };

  static constexpr std::array<std::string_view, 7> kSansSerifCandidates = {
      "DejaVu Sans", "Liberation Sans", "Noto Sans", "FreeSans",
      "Arial",       "Helvetica",       "ABeeZee",
  };
  static constexpr std::array<std::string_view, 7> kSerifCandidates = {
      "DejaVu Serif", "Liberation Serif", "Noto Serif",
      "FreeSerif",    "Times New Roman",  "Times",
      "ABeeZee",
  };
  static constexpr std::array<std::string_view, 7> kMonospaceCandidates = {
      "DejaVu Sans Mono", "Liberation Mono", "Noto Sans Mono", "FreeMono",
      "Courier New",      "Courier",         "ABeeZee",
  };

  BLFontFace resolved_face;
  for (const auto &family : spec.families) {
    if (EqualsIgnoreCase(family, "sans-serif") ||
        EqualsIgnoreCase(family, "system-ui") ||
        EqualsIgnoreCase(family, "cursive") ||
        EqualsIgnoreCase(family, "fantasy")) {
      for (std::string_view candidate : kSansSerifCandidates) {
        if (QuerySingleFamily(candidate, props, resolved_face)) {
          break;
        }
      }
    } else if (EqualsIgnoreCase(family, "serif")) {
      for (std::string_view candidate : kSerifCandidates) {
        if (QuerySingleFamily(candidate, props, resolved_face)) {
          break;
        }
      }
    } else if (EqualsIgnoreCase(family, "monospace")) {
      for (std::string_view candidate : kMonospaceCandidates) {
        if (QuerySingleFamily(candidate, props, resolved_face)) {
          break;
        }
      }
    } else {
      QuerySingleFamily(family, props, resolved_face);
    }

    if (resolved_face.is_valid()) {
      break;
    }
  }

  if (!resolved_face.is_valid()) {
    resolved_face = fallback_face_;
  }

  BLFont font;
  font.create_from_face(resolved_face, spec.size_px);
  return font;
}

size_t FontRegistry::LoadedFaceCount() const noexcept {
  return manager_.face_count();
}

size_t FontRegistry::LoadedFamilyCount() const noexcept {
  return manager_.family_count();
}

}  // namespace canvas
}  // namespace quickps
