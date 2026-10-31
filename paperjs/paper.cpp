#include <paperjs/paper.h>

namespace {
constexpr uint8_t kByteCode[] = {
#embed "paper.bc"
};
} // namespace

namespace paper {
std::span<const uint8_t> GetByteCode() { return kByteCode; }
} // namespace paper
