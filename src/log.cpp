#include "log.h"

namespace logger {

std::mutex file_lock;
std::ofstream log_f;
LogLevel log_level = LogLevel::INFO;

void init(const std::string& path) {
  log_f.open(path, std::ios::out | std::ios::app);
}

void set_level(const LogLevel& level) { log_level = level; }

static int level_int(const LogLevel& level) {
  switch (level) {
    case LogLevel::DEBUG:
      return 4;
    case LogLevel::INFO:
      return 3;
    case LogLevel::WARNING:
      return 2;
    case LogLevel::ERROR:
      return 1;
    default:
      return 999;
  }
}

void log(const LogLevel& level, const std::string& line) {
  if (level_int(level) > level_int(log_level)) {
    return;
  }
  std::lock_guard<std::mutex> guard(file_lock);
  const auto now =
      std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  log_f << std::put_time(std::localtime(&now), "%D %H:%M:%S") << " "
        << static_cast<char>(level) << " " << line << std::endl;
}

void debug(const std::string& line) { log(LogLevel::DEBUG, line); }
void info(const std::string& line) { log(LogLevel::INFO, line); }
void warning(const std::string& line) { log(LogLevel::WARNING, line); }
void error(const std::string& line) { log(LogLevel::ERROR, line); }

void cleanup() { log_f.close(); }

}  // namespace log
