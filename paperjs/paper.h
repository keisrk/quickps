#ifndef PAPER_H_
#define PAPER_H_

#include <cstdint>
#include <span>

namespace paper {
std::span<const uint8_t> GetByteCode();
} // namespace paper

#endif // PAPER_H_
