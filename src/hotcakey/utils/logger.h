#ifndef HOTCAKEY_UTILS_LOGGER_H_
#define HOTCAKEY_UTILS_LOGGER_H_

#include <iostream>
#include <sstream>
#include <string>

#define LOG(msg)                                                           \
  {                                                                        \
    std::stringstream buffer;                                              \
    buffer << "[hotcakey:dbg] " << msg << " (" __FILE__ << ":" << __LINE__ \
           << ")";                                                         \
    hotcakey::utils::Log(buffer.str());                                    \
  }

#define WRN(msg)                                                           \
  {                                                                        \
    std::stringstream buffer;                                              \
    buffer << "[hotcakey:wrn] " << msg << " (" __FILE__ << ":" << __LINE__ \
           << ")";                                                         \
    hotcakey::utils::Err(buffer.str());                                    \
  }

#define ERR(msg)                                                           \
  {                                                                        \
    std::stringstream buffer;                                              \
    buffer << "[hotcakey:err] " << msg << " (" __FILE__ << ":" << __LINE__ \
           << ")";                                                         \
    hotcakey::utils::Err(buffer.str());                                    \
  }

namespace hotcakey {
namespace utils {

void SetVerbose(const bool isVerbose);
void Log(const std::string& msg);
void Wrn(const std::string& msg);
void Err(const std::string& msg);

}  // namespace utils
}  // namespace hotcakey

#endif  // HOTCAKEY_UTILS_LOGGER_H_
