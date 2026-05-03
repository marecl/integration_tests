#include "log.h"

#include <iomanip>
#include <sstream>

namespace Style {
static const unsigned style_size = __enum_end + 1;

static const char* styles[style_size] = {
    "\033[0m",  "\033[1m",                                                                          // res, bold
    "\033[22m",                                                                                     // notbold
    "\033[30m", "\033[31m", "\033[32m", "\033[33m", "\033[34m", "\033[35m", "\033[36m", "\033[37m", // FG
    "",                                                                                             // end
};

} // namespace Style

const char* GetSt(Style::Style style) {
  using namespace Style;
  if (style < style_size) return styles[style];
  return styles[style_size - 1];
}

std::ostream& center(std::ostream& os, const std::string& s, int width) {
  int len = (int)s.size();
  if (width <= len) return os << s.substr(0, width);
  int left  = (width - len) / 2;
  int right = width - len - left;
  return os << std::string(left, ' ') << s << std::string(right, ' ');
}

std::string center(const std::string& s, int width) {
  int len = (int)s.size();
  if (width <= len) return s.substr(0, width);
  int left  = (width - len) / 2;
  int right = width - len - left;
  return std::string(std::string(left, ' ') + s + std::string(right, ' '));
}

std::ostream& right(std::ostream& os, const std::string& s, int width) {
  int len = (int)s.size();
  if (width <= len) return os << s.substr(0, width);
  int left = (width - len);
  return os << std::string(left, ' ') << s;
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