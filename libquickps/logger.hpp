#pragma once

#include <print>
#include <string_view>

namespace quickps {

enum class LogLevel {
  Debug = 0,
  Info = 1,
  Warn = 2,
  Error = 3,
  Off = 4
};

class Logger {
public:
  static Logger &instance() {
    static Logger logger;
    return logger;
  }

  void set_level(LogLevel level) noexcept { level_ = level; }
  LogLevel level() const noexcept { return level_; }

private:
  // Default: Warn (silent unless something bad happens)
  LogLevel level_ = LogLevel::Warn;
};

namespace log {

template <typename... Args>
inline void debug(std::format_string<Args...> fmt, Args &&...args) {
  if (Logger::instance().level() <= LogLevel::Debug) {
    std::println(stderr, "[DEBUG] {}",
                 std::format(fmt, std::forward<Args>(args)...));
  }
}

inline void debug(std::string_view msg) {
  if (Logger::instance().level() <= LogLevel::Debug) {
    std::println(stderr, "[DEBUG] {}", msg);
  }
}

template <typename... Args>
inline void info(std::format_string<Args...> fmt, Args &&...args) {
  if (Logger::instance().level() <= LogLevel::Info) {
    std::println(stderr, "[INFO]  {}",
                 std::format(fmt, std::forward<Args>(args)...));
  }
}

inline void info(std::string_view msg) {
  if (Logger::instance().level() <= LogLevel::Info) {
    std::println(stderr, "[INFO]  {}", msg);
  }
}

template <typename... Args>
inline void warn(std::format_string<Args...> fmt, Args &&...args) {
  if (Logger::instance().level() <= LogLevel::Warn) {
    std::println(stderr, "[WARN]  {}",
                 std::format(fmt, std::forward<Args>(args)...));
  }
}

inline void warn(std::string_view msg) {
  if (Logger::instance().level() <= LogLevel::Warn) {
    std::println(stderr, "[WARN]  {}", msg);
  }
}

template <typename... Args>
inline void error(std::format_string<Args...> fmt, Args &&...args) {
  if (Logger::instance().level() <= LogLevel::Error) {
    std::println(stderr, "[ERROR] {}",
                 std::format(fmt, std::forward<Args>(args)...));
  }
}

inline void error(std::string_view msg) {
  if (Logger::instance().level() <= LogLevel::Error) {
    std::println(stderr, "[ERROR] {}", msg);
  }
}

} // namespace log
} // namespace quickps
