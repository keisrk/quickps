#include <libquickps/canvas/image.hpp>

namespace quickps {
namespace canvas {

namespace {

const char kBase64Chars[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

}  // namespace

std::string Base64Encode(const uint8_t *data, size_t len) {
  std::string out;
  out.reserve(((len + 2) / 3) * 4);
  for (size_t i = 0; i < len; i += 3) {
    uint32_t val = static_cast<uint32_t>(data[i]) << 16;
    if (i + 1 < len) val |= static_cast<uint32_t>(data[i + 1]) << 8;
    if (i + 2 < len) val |= static_cast<uint32_t>(data[i + 2]);

    out.push_back(kBase64Chars[(val >> 18) & 0x3F]);
    out.push_back(kBase64Chars[(val >> 12) & 0x3F]);
    out.push_back((i + 1 < len) ? kBase64Chars[(val >> 6) & 0x3F] : '=');
    out.push_back((i + 2 < len) ? kBase64Chars[val & 0x3F] : '=');
  }
  return out;
}

std::vector<uint8_t> Base64Decode(std::string_view in) {
  std::vector<uint8_t> out;
  std::vector<int> table(256, -1);
  for (int i = 0; i < 64; i++) {
    table[static_cast<uint8_t>(kBase64Chars[i])] = i;
  }

  uint32_t val = 0;
  int valb = -8;
  for (uint8_t c : in) {
    if (table[c] == -1) {
      if (c == '=') break;
      continue;
    }
    val = (val << 6) | table[c];
    valb += 6;
    if (valb >= 0) {
      out.push_back(static_cast<uint8_t>((val >> valb) & 0xFF));
      valb -= 8;
    }
  }
  return out;
}

Image::Image() = default;

Image::Image(int width, int height) : width_(width), height_(height) {}

bool Image::LoadFromFile(const std::filesystem::path &path) {
  src_ = path.string();
  auto result = surface_.read_from_file(src_.c_str());
  complete_ = (result == BL_SUCCESS);
  return complete_;
}

void Image::SetSrc(const std::string &src) {
  src_ = src;
  if (src_.rfind("data:", 0) == 0) {
    auto comma = src_.find(',');
    if (comma != std::string::npos) {
      auto payload = std::string_view(src_).substr(comma + 1);
      auto decoded = Base64Decode(payload);
      if (!decoded.empty()) {
        surface_.read_from_data(decoded.data(), decoded.size());
      }
    }
    complete_ = true;
  } else {
    // Direct filesystem access via src assignment is disallowed from JavaScript.
    // Images must either be inline data: URLs or loaded via load(path) / pre-registered.
    surface_.reset();
    complete_ = false;
  }
}

int Image::width() const {
  if (width_ > 0) return width_;
  return surface_.width();
}

void Image::SetWidth(int w) { width_ = w; }

int Image::height() const {
  if (height_ > 0) return height_;
  return surface_.height();
}

void Image::SetHeight(int h) { height_ = h; }

int Image::naturalWidth() const { return surface_.width(); }

int Image::naturalHeight() const { return surface_.height(); }

}  // namespace canvas

namespace quickjs {

namespace {

Value GetSrc(Context &ctx, canvas::Image *img) { return ctx.Get(img->src()); }

void SetSrc(Context &ctx, canvas::Image *img, Value v) {
  img->SetSrc(v.Get<std::string>(ctx));
}

Value GetWidth(Context &ctx, canvas::Image *img) {
  return ctx.Get(img->width());
}

void SetWidth(Context &ctx, canvas::Image *img, Value v) {
  img->SetWidth(v.Get<int>(ctx));
}

Value GetHeight(Context &ctx, canvas::Image *img) {
  return ctx.Get(img->height());
}

void SetHeight(Context &ctx, canvas::Image *img, Value v) {
  img->SetHeight(v.Get<int>(ctx));
}

Value GetNaturalWidth(Context &ctx, canvas::Image *img) {
  return ctx.Get(img->naturalWidth());
}

Value GetNaturalHeight(Context &ctx, canvas::Image *img) {
  return ctx.Get(img->naturalHeight());
}

Value GetComplete(Context &ctx, canvas::Image *img) {
  return ctx.Get(img->complete());
}

Value GetCrossOrigin(Context &ctx, canvas::Image *img) {
  return ctx.Get(img->crossOrigin());
}

void SetCrossOrigin(Context &ctx, canvas::Image *img, Value v) {
  img->SetCrossOrigin(v.Get<std::string>(ctx));
}

Value AddEventListener([[maybe_unused]] Context &ctx,
                       [[maybe_unused]] canvas::Image *img,
                       [[maybe_unused]] const ArgumentList &args) {
  return Value::GetUndefined();
}

Value Load(Context &ctx, canvas::Image *img, const ArgumentList &args) {
  if (args.empty()) return ctx.Get(false);
  std::string path = args[0].Get<std::string>(ctx);
  return ctx.Get(img->LoadFromFile(path));
}

}  // namespace

template <>
const Class &GetClass<canvas::Image>() {
  static Class kClass = ClassBuilder<canvas::Image>("Image")
                            .Constructor<New<canvas::Image>>(2)
                            .Proto(EntryBuilder<canvas::Image>()
                                       .Name("src")
                                       .Getter<GetSrc>()
                                       .Setter<SetSrc>()
                                       .Build())
                            .Proto(EntryBuilder<canvas::Image>()
                                       .Name("width")
                                       .Getter<GetWidth>()
                                       .Setter<SetWidth>()
                                       .Build())
                            .Proto(EntryBuilder<canvas::Image>()
                                       .Name("height")
                                       .Getter<GetHeight>()
                                       .Setter<SetHeight>()
                                       .Build())
                            .Proto(EntryBuilder<canvas::Image>()
                                       .Name("naturalWidth")
                                       .Getter<GetNaturalWidth>()
                                       .Build())
                            .Proto(EntryBuilder<canvas::Image>()
                                       .Name("naturalHeight")
                                       .Getter<GetNaturalHeight>()
                                       .Build())
                            .Proto(EntryBuilder<canvas::Image>()
                                       .Name("complete")
                                       .Getter<GetComplete>()
                                       .Build())
                            .Proto(EntryBuilder<canvas::Image>()
                                       .Name("crossOrigin")
                                       .Getter<GetCrossOrigin>()
                                       .Setter<SetCrossOrigin>()
                                       .Build())
                            .Proto(EntryBuilder<canvas::Image>()
                                       .Name("addEventListener")
                                       .Method<AddEventListener>(2)
                                       .Build())
                            .Proto(EntryBuilder<canvas::Image>()
                                       .Name("load")
                                       .Method<Load>(1)
                                       .Build())
                            .Build();

  return kClass;
}

template <>
std::unique_ptr<canvas::Image, OpaqueDeleter<canvas::Image>> New<canvas::Image>(
    Context &ctx, ValueIter first, ValueIter last) {
  int w = 0, h = 0;
  if (first != last) w = first++->Get<int>(ctx);
  if (first != last) h = first++->Get<int>(ctx);
  return Runtime::GetInstance().Ctor<canvas::Image>(w, h);
}

}  // namespace quickjs
}  // namespace quickps
