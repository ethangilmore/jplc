#include "logger.h"
#include <iostream>

Logger::Logger(std::string& filename) : filename(filename) {
  file.open(filename);
}

Logger::~Logger() {
  file.close();
}

[[noreturn]] void Logger::log_error(std::string message, uint64_t position) {
  auto [line, col] = get_line_col(position);
  std::cout << "Compilation failed: " << filename << "[" << line << ":" << col
            << "]: " << message << std::endl;
  exit(1);
}

std::pair<uint64_t, uint64_t> Logger::get_line_col(uint64_t position) {
  file.seekg(0);
  uint64_t line = 1;
  uint64_t col = 1;
  for (uint64_t i = 0; i < position; i++) {
    char c = file.get();
    if (c == '\n') {
      line++;
      col = 1;
    } else {
      col++;
    }
  }
  return {line, col};
}
