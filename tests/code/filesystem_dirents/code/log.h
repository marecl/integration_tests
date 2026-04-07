#pragma once

#ifndef LOG_H
#define LOG_H

#include <iomanip>
#include <iostream>
#include <sstream>

#define STR(x) std::to_string(x)

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
  return oss.str();
}

std::string to_hex_string(const void* data, long long length, std::string sep=" ");

template <typename... Args>
void LogCustom(const char* fn, const char* msg, Args&&... args) {
  std::cout << "[" << center(fn, 20) << "] " << msg;
  ((std::cout << " " << args), ...);
  std::cout << std::endl;
}

extern int error_counter;

int  GetErrorCounter(void);
void ResetErrorCounter(void);

#define Log(...)                                                                                                                                               \
  {                                                                                                                                                            \
    LogCustom(__FUNCTION__, "[INFO]", ##__VA_ARGS__);                                                                                                          \
  }

#define LogTest(...)                                                                                                                                           \
  {                                                                                                                                                            \
    LogCustom(__FUNCTION__, "\033[34;1m[TEST]\033[0m", ##__VA_ARGS__);                                                                                         \
  }

#define LogError(...)                                                                                                                                          \
  {                                                                                                                                                            \
    LogCustom(__FUNCTION__, "\033[31;1m[FAIL]\033[0m", ##__VA_ARGS__, "( " __FILE__ ":", __LINE__, ")");                                                       \
  }

#define LogWarning(...)                                                                                                                                        \
  {                                                                                                                                                            \
    LogCustom(__FUNCTION__, "\033[33;1m[WARN]\033[0m", ##__VA_ARGS__, "( " __FILE__ ":", __LINE__, ")");                                                       \
  }

#define LogSuccess(...)                                                                                                                                        \
  {                                                                                                                                                            \
    LogCustom(__FUNCTION__, "\033[32;1m[SUCC]\033[0m", ##__VA_ARGS__);                                                                                         \
  }

#define TEST_CASE(cond, success_str, fail_str, ...)                                                                                                            \
  {                                                                                                                                                            \
    if (cond) {                                                                                                                                                \
      LogSuccess(success_str, ##__VA_ARGS__);                                                                                                                  \
    } else {                                                                                                                                                   \
      LogError(fail_str, ##__VA_ARGS__);                                                                                                                       \
    }                                                                                                                                                          \
  }

#endif