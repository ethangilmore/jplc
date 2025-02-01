#pragma once

#include <cstdint>
#include <fstream>

class Logger {
 public:
  Logger(std::string& filename);
  ~Logger();
  void log_error(std::string message, uint64_t position);
  std::pair<uint64_t, uint64_t> get_line_col(uint64_t position);

 private:
  std::string filename;
  std::ifstream file;
};
