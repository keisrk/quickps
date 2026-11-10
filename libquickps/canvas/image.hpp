#ifndef QUICKPS_CANVAS_IMAGE_H_
#define QUICKPS_CANVAS_IMAGE_H_

#include <blend2d/blend2d.h>

#include <filesystem>
#include <libquickps/quickjsmm/esm.hpp>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace quickps {
namespace canvas {

std::string Base64Encode(const uint8_t *data, size_t len);
std::vector<uint8_t> Base64Decode(std::string_view in);

class Image final {
 public:
  Image();
  explicit Image(int width, int height);
  ~Image() = default;

  const BLImage &Impl() const { return surface_; }
  BLImage &Impl() { return surface_; }

  const std::string &src() const { return src_; }
  void SetSrc(const std::string &src);
  bool LoadFromFile(const std::filesystem::path &path);

  int width() const;
  void SetWidth(int w);

  int height() const;
  void SetHeight(int h);

  int naturalWidth() const;
  int naturalHeight() const;

  bool complete() const { return complete_; }

  const std::string &crossOrigin() const { return cross_origin_; }
  void SetCrossOrigin(const std::string &co) { cross_origin_ = co; }

 private:
  BLImage surface_;
  std::string src_;
  std::string cross_origin_;
  int width_ = 0;
  int height_ = 0;
  bool complete_ = true;
};

}  // namespace canvas

namespace quickjs {

template <>
const Class &GetClass<canvas::Image>();

template <>
std::unique_ptr<canvas::Image, OpaqueDeleter<canvas::Image>> New<canvas::Image>(
    Context &ctx, ValueIter first, ValueIter last);

}  // namespace quickjs
}  // namespace quickps

#endif  // QUICKPS_CANVAS_IMAGE_H_
