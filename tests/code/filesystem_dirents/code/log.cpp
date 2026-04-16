#include "log.h"

#include <iomanip>
#include <sstream>

std::ostream& center(std::ostream& os, const std::string& s, int width) {
  int len = (int)s.size();
  if (width <= len) return os << s.substr(0, width);
  int left  = (width - len) / 2;
  int right = width - len - left;
  return os << std::string(left, ' ') << s << std::string(right, ' ');
}

std::ostream& right(std::ostream& os, const std::string& s, int width) {
  int len = (int)s.size();
  if (width <= len) return os << s.substr(0, width);
  int left = (width - len);
  return os << std::string(left, ' ') << s;
}

std::string center(const std::string& s, int width) {
  int len = (int)s.size();
  if (width <= len) return s.substr(0, width);
  int left  = (width - len) / 2;
  int right = width - len - left;
  return std::string(std::string(left, ' ') + s + std::string(right, ' '));
}

std::string right(const std::string& s, int width) {
  int len = (int)s.size();
  if (width <= len) return s.substr(0, width);
  int left = (width - len);
  return std::string(std::string(left, ' ') + s);
}

std::string to_hex_string(const void* data, long long length, std::string sep) {
  std::ostringstream oss {};
  for (auto i = 0; i < length; i++) {
    oss << std::hex << std::setw(2) << std::setfill('0') << (0xFF & static_cast<unsigned int>(*(reinterpret_cast<const char*>(data) + i))) << sep;
  }
  return oss.str();
}