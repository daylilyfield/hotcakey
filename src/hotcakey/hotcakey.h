#ifndef HOTCAKEY_H_
#define HOTCAKEY_H_

#include <ctime>
#include <string>
#include <vector>

namespace hotcakey {

enum Result { kSuccess, kFailure };

Result Activate();
Result Inactivate();

inline std::string ToString(Result result) {
  switch (result) {
    case kSuccess:
      return "success";
    case kFailure:
      return "failure";
  }
}

enum EventType { kKeyDown, kKeyUp };

using Registration = unsigned long;
using RegistrationResult = std::pair<Result, Registration>;

struct Event {
  EventType type;
  std::time_t time;
  Event(EventType type, std::time_t time) : type(type), time(time){};
};

RegistrationResult Register(const std::vector<std::string>& keys,
                            const std::function<void(Event)>& listener);
Result Unregister(const Registration& registration);

inline std::string ToString(EventType type) {
  switch (type) {
    case kKeyDown:
      return "keydown";
    case kKeyUp:
      return "keyup";
  }
}

}  // namespace hotcakey

#endif  // HOTCAKEY_H_
