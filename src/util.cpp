#include "util.h"

namespace util {

  std::shared_ptr<spdlog::logger> out() {
    static auto logger = spdlog::stdout_logger_mt("stdout", true);
    return logger;
  }

  std::shared_ptr<spdlog::logger> err() {
    static auto logger = spdlog::stderr_logger_mt("stderr", true);
    return logger;
  }

}  // namespace util
