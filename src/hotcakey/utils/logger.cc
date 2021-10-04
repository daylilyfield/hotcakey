#include "./logger.h"

#include <atomic>

namespace {

std::atomic<bool> isVerbose(false);

}  // namespace

namespace hotcakey {
namespace utils {

void SetVerbose(bool verbose) {
  isVerbose.store(verbose, std::memory_order_release);
}

void Log(const std::string& msg) {
  if (isVerbose.load(std::memory_order_acquire)) {
    std::cout << msg << std::endl;
  }
}

void Wrn(const std::string& msg) { std::cerr << msg << std::endl; }

void Err(const std::string& msg) { std::cerr << msg << std::endl; }

}  // namespace utils
}  // namespace hotcakey
