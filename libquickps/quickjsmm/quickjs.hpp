#ifndef QUICKPS_QUICKJSMM_QUICKJS_H_
#define QUICKPS_QUICKJSMM_QUICKJS_H_

#include <exception>
#include <string>
#include <vector>

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
  virtual JSContext *cobj() = 0;
  virtual ~ContextProvider() = default;
};

class Entry final {
public:
  Entry(JSCFunctionListEntry entry) : cobj_(entry) {}
  JSCFunctionListEntry &cobj() { return cobj_; }

private:
  JSCFunctionListEntry cobj_;
};

struct Class {
  JSClassDef def;
  JSCFunction *ctor;
  size_t ctor_argc;
  std::vector<JSCFunctionListEntry> prototype;
};

class EsModule final {
public:
  const char *name() { return name_.c_str(); }

private:
  std::string name_;
};

} // namespace quickjs
} // namespace quickps

#endif // QUICKPS_QUICKJSMM_QUICKJS_H_
