#include <vector>
#include <string>
#include <sstream>

using namespace std;

namespace hotcakey {
  vector<string> split(const string &s, char delim) {
      vector<string> elems;
      stringstream ss(s);
      string item;
      while (getline(ss, item, delim)) {
      if (!item.empty()) {
              elems.push_back(item);
          }
      }
      return elems;
  }
}
