#include "./strings.h"

namespace hotcakey {
namespace utils {

std::vector<std::string> Split(const std::string& s, char delim) {
  std::vector<std::string> elems;
  std::stringstream ss(s);
  std::string item;
  while (getline(ss, item, delim)) {
    if (!item.empty()) {
      elems.push_back(item);
    }
  }
  return elems;
}

std::string Join(const std::vector<std::string>& v, const char* delim) {
  std::string s;
  if (!v.empty()) {
    s += v[0];
    for (decltype(v.size()) i = 1, c = v.size(); i < c; ++i) {
      if (delim) s += delim;
      s += v[i];
    }
  }
  return s;
}

std::string ToLower(const std::string& v) {
  auto s = std::string(v);
  std::transform(s.begin(), s.end(), s.begin(), ::tolower);
  return s;
}

}  // namespace utils
}  // namespace hotcakey
