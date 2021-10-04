#ifndef HOTCAKEY_UTILS_STRINGS_H_
#define HOTCAKEY_UTILS_STRINGS_H_

#include <sstream>
#include <string>
#include <vector>

namespace hotcakey {
namespace utils {

std::vector<std::string> Split(const std::string& s, char delim);
std::string Join(const std::vector<std::string>& v, const char* delim = 0);
std::string ToLower(const std::string& s);

}  // namespace utils
}  // namespace hotcakey

#endif  // HOTCAKEY_UTILS_STRINGS_H_
