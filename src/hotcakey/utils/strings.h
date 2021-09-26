#ifndef HOTCAKEY_UTILS_STRINGS_H_
#define HOTCAKEY_UTILS_STRINGS_H_

#include <vector>
#include <string>
#include <sstream>

namespace hotcakey {
namespace utils {

  std::vector<std::string> Split(const std::string& s, char delim);
  std::string Join(const std::vector<std::string>& v, const char* delim = 0);
  std::string ToLower(const std::string& s);

} // utils
} // hotcakey

#endif // HOTCAKEY_UTILS_STRINGS_H_
