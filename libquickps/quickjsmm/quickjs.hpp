#ifndef QUICKPS_QUICKJSMM_QUICKJS_H_
#define QUICKPS_QUICKJSMM_QUICKJS_H_

#include <exception>

extern "C" {
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpedantic"
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wcast-function-type"
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif

#include <quickjs-libc.h>

#if defined(__clang__)
#pragma clang diagnostic pop
#else
#pragma GCC diagnostic pop
#endif
}

namespace quickps {
namespace quickjs {

class Exception : public std::exception {};

class ContextProvider {
public:
  virtual JSContext *GetInstance() = 0;
  virtual ~ContextProvider() = default;
};

} // namespace quickjs
} // namespace quickps

#endif // QUICKPS_QUICKJSMM_QUICKJS_H_
