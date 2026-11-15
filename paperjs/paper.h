#ifndef PAPER_H_
#define PAPER_H_

#include <cstdint>
#include <span>
#include <string_view>

namespace paper {
std::span<const uint8_t> GetByteCode();
std::string_view GetSource();
}  // namespace paper

#endif  // PAPER_H_
