#ifndef LOG_H
#define LOG_H

#pragma once

#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <utility>

#define STR(x) std::to_string(x)

namespace Style {
typedef enum Style: unsigned {
  RESET = 0,
  BOLD,
  NOTBOLD,
  FG_BLACK,
  FG_RED,
  FG_GREEN,
  FG_YELLOW,
  FG_BLUE,
  FG_MAGENTA,
  FG_CYAN,
  FG_WHITE,
  __enum_end,
} Style;

} // namespace Style

const char* GetSt(Style::Style code);

std::ostream& center(std::ostream& os, const std::string& s, int width);
std::string   center(const std::string& s, int width);
std::ostream& right(std::ostream& os, const std::string& s, int width);
std::string   right(const std::string& s, int width);

template <typename T>
std::string to_octal(T value) {
  std::ostringstream oss;
  oss << std::oct << value;
  return oss.str();
}

template <typename T>
std::string to_hex(T value) {
  std::ostringstream oss;
  oss << std::hex << value;
  return std::string(oss.str());
}

std::string to_hex_string(const void* data, long long length, std::string sep = " ");

void LogInit(std::filesystem::path);
void LogEnd();

extern std::fstream g_log;

template <typename... Args>
void LogCustom(const char* fn, bool show_line, const char* msg, Args&&... args) {
  if (!g_log.is_open()) return;

  std::ostringstream tmp;
  tmp << GetSt(Style::RESET) << '[' << center(fn, 20) << "] " << msg;
  ((tmp << ' ' << args), ...);
  tmp << GetSt(Style::RESET);

  if (show_line) {
    tmp << " ( " << __FILE__ << ':' << __LINE__ << " )";
  }
  tmp << std::endl;

  const auto& stmp = tmp.str();
  g_log << stmp;
  std::cout << stmp;
}

extern int error_counter;

int  GetErrorCounter(void);
void ResetErrorCounter(void);

#define Log(...)                                                                                                                                               \
  {                                                                                                                                                            \
    LogCustom(__FUNCTION__, false, "  [INFO]", ##__VA_ARGS__);                                                                                                 \
  }

#define LogTest(...)                                                                                                                                           \
  {                                                                                                                                                            \
    LogCustom(__FUNCTION__, false, GetSt(Style::BOLD), GetSt(Style::FG_BLUE), "[TEST]", GetSt(Style::RESET), ##__VA_ARGS__);                                   \
  }

#define LogError(...)                                                                                                                                          \
  {                                                                                                                                                            \
    LogCustom(__FUNCTION__, true, GetSt(Style::BOLD), GetSt(Style::FG_RED), "[FAIL]", GetSt(Style::RESET), ##__VA_ARGS__);                                     \
  }

#define LogWarning(...)                                                                                                                                        \
  {                                                                                                                                                            \
    LogCustom(__FUNCTION__, true, GetSt(Style::BOLD), GetSt(Style::FG_YELLOW), "[WARN]", GetSt(Style::RESET), ##__VA_ARGS__);                                  \
  }

#define LogSuccess(...)                                                                                                                                        \
  {                                                                                                                                                            \
    LogCustom(__FUNCTION__, false, GetSt(Style::BOLD), GetSt(Style::FG_GREEN), "[SUCC]", GetSt(Style::RESET), ##__VA_ARGS__);                                  \
  }

#define TEST_CASE(cond, success_str, fail_str, ...)                                                                                                            \
  {                                                                                                                                                            \
    if (cond) {                                                                                                                                                \
      LogSuccess(success_str, ##__VA_ARGS__);                                                                                                                  \
    } else {                                                                                                                                                   \
      LogError(fail_str, ##__VA_ARGS__);                                                                                                                       \
    }                                                                                                                                                          \
  }

#endif // LOG_H