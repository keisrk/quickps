#include <paperjs/paper.h>

namespace {
constexpr char kSource[] = {
#embed "paper-core.esm"
    , '\0'};
}  // namespace

namespace paper {
std::string_view GetSource() { return {kSource, sizeof(kSource) - 1}; }
}  // namespace paper
