#include "./hotcakey.h"

#include <windows.h>

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <ctime>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include "./utils/logger.h"
#include "./utils/strings.h"

namespace {

struct Listener {
  hotcakey::Registration registration;
  std::function<void(hotcakey::Event)> callback;
};

struct RegistrationRequest {
  hotcakey::Registration registration;
  UINT modifiers;
  UINT key;
};

std::thread nativeThread;

// why do we use `atomic<bool> instead of `bool with mutex`?
// because this variable is read in the Carbon Event Loop,
// we should not repeat to lock and release mutext for performance reason.
std::atomic<bool> isActive(false);

std::unordered_map<hotcakey::Registration, const Listener*> listeners;

std::vector<RegistrationRequest*> requests;

std::mutex mutex;
std::condition_variable cond;

hotcakey::Registration eventHotKeyIdSequence = 0;

constexpr unsigned long long int Hash(const char* str,
                                      unsigned long long int hash = 0) {
  return (*str == 0) ? hash : 101 * Hash(str + 1) + *str;
}

UINT MapWinVirtualKey(const std::string& key) {
  auto ch = key.c_str();
  UINT scancode = 0;

  switch (Hash(ch)) {
    // modifiers key to ignore
    case Hash("Control"):
    case Hash("ControlRight"):
    case Hash("ControlLeft"):
    case Hash("Shift"):
    case Hash("ShiftRight"):
    case Hash("ShiftLeft"):
    case Hash("Alt"):
    case Hash("AltRight"):
    case Hash("AltLeft"):
    case Hash("Meta"):
    case Hash("MetaRight"):
    case Hash("MetaLeft"):
      return UINT32_MAX;

    case Hash("KeyA"):
      scancode = 0x001E;
      break;
    case Hash("KeyB"):
      scancode = 0x0030;
      break;
    case Hash("KeyC"):
      scancode = 0x002E;
      break;
    case Hash("KeyD"):
      scancode = 0x0020;
      break;
    case Hash("KeyE"):
      scancode = 0x0012;
      break;
    case Hash("KeyF"):
      scancode = 0x0021;
      break;
    case Hash("KeyG"):
      scancode = 0x0022;
      break;
    case Hash("KeyH"):
      scancode = 0x0023;
      break;
    case Hash("KeyI"):
      scancode = 0x0017;
      break;
    case Hash("KeyJ"):
      scancode = 0x0024;
      break;
    case Hash("KeyK"):
      scancode = 0x0025;
      break;
    case Hash("KeyL"):
      scancode = 0x0026;
      break;
    case Hash("KeyM"):
      scancode = 0x0032;
      break;
    case Hash("KeyN"):
      scancode = 0x0031;
      break;
    case Hash("KeyO"):
      scancode = 0x0018;
      break;
    case Hash("KeyP"):
      scancode = 0x0019;
      break;
    case Hash("KeyQ"):
      scancode = 0x0010;
      break;
    case Hash("KeyR"):
      scancode = 0x0013;
      break;
    case Hash("KeyS"):
      scancode = 0x001F;
      break;
    case Hash("KeyT"):
      scancode = 0x0014;
      break;
    case Hash("KeyU"):
      scancode = 0x0016;
      break;
    case Hash("KeyV"):
      scancode = 0x002F;
      break;
    case Hash("KeyW"):
      scancode = 0x0011;
      break;
    case Hash("KeyX"):
      scancode = 0x002D;
      break;
    case Hash("KeyY"):
      scancode = 0x0015;
      break;
    case Hash("KeyZ"):
      scancode = 0x002C;
      break;
    case Hash("Digit1"):
      scancode = 0x0002;
      break;
    case Hash("Digit2"):
      scancode = 0x0003;
      break;
    case Hash("Digit3"):
      scancode = 0x0004;
      break;
    case Hash("Digit4"):
      scancode = 0x0005;
      break;
    case Hash("Digit5"):
      scancode = 0x0006;
      break;
    case Hash("Digit6"):
      scancode = 0x0007;
      break;
    case Hash("Digit7"):
      scancode = 0x0008;
      break;
    case Hash("Digit8"):
      scancode = 0x0009;
      break;
    case Hash("Digit9"):
      scancode = 0x000A;
      break;
    case Hash("Digit0"):
      scancode = 0x000B;
      break;
    case Hash("Minus"):
      scancode = 0x000C;
      break;
    case Hash("Equal"):
      scancode = 0x000D;
      break;
    case Hash("BracketLeft"):
      scancode = 0x001A;
      break;
    case Hash("BracketRight"):
      scancode = 0x001B;
      break;
    case Hash("Backslash"):
      scancode = 0x002B;
      break;
    case Hash("Semicolon"):
      scancode = 0x0027;
      break;
    case Hash("Quote"):
      scancode = 0x0028;
      break;
    case Hash("Backquote"):
      scancode = 0x0029;
      break;
    case Hash("Comma"):
      scancode = 0x0033;
      break;
    case Hash("Period"):
      scancode = 0x0034;
      break;
    case Hash("Slash"):
      scancode = 0x0035;
      break;
    case Hash("Enter"):
      scancode = 0x001C;
      break;
    case Hash("Escape"):
      scancode = 0x0001;
      break;
    case Hash("Backspace"):
      scancode = 0x000E;
      break;
    case Hash("Tab"):
      scancode = 0x000F;
      break;
    case Hash("Space"):
      scancode = 0x0039;
      break;
    case Hash("CapsLock"):
      scancode = 0x003A;
      break;
    case Hash("F1"):
      scancode = 0x003B;
      break;
    case Hash("F2"):
      scancode = 0x003C;
      break;
    case Hash("F3"):
      scancode = 0x003D;
      break;
    case Hash("F4"):
      scancode = 0x003E;
      break;
    case Hash("F5"):
      scancode = 0x003F;
      break;
    case Hash("F6"):
      scancode = 0x0040;
      break;
    case Hash("F7"):
      scancode = 0x0041;
      break;
    case Hash("F8"):
      scancode = 0x0042;
      break;
    case Hash("F9"):
      scancode = 0x0043;
      break;
    case Hash("F10"):
      scancode = 0x0044;
      break;
    case Hash("F11"):
      scancode = 0x0057;
      break;
    case Hash("F12"):
      scancode = 0x0058;
      break;
    case Hash("F13"):
      scancode = 0x0064;
      break;
    case Hash("F14"):
      scancode = 0x0065;
      break;
    case Hash("F15"):
      scancode = 0x0066;
      break;
    case Hash("F16"):
      scancode = 0x0067;
      break;
    case Hash("F17"):
      scancode = 0x0068;
      break;
    case Hash("F18"):
      scancode = 0x0069;
      break;
    case Hash("F19"):
      scancode = 0x006A;
      break;
    case Hash("F20"):
      scancode = 0x006B;
      break;
    case Hash("F21"):
      scancode = 0x006C;
      break;
    case Hash("F22"):
      scancode = 0x006D;
      break;
    case Hash("F23"):
      scancode = 0x006E;
      break;
    case Hash("F24"):
      scancode = 0x0076;
      break;
    case Hash("PrintScreen"):
      scancode = 0xE037;
      break;
    case Hash("ScrollLock"):
      scancode = 0x0046;
      break;
    case Hash("Pause"):
      scancode = 0x0045;
      break;
    case Hash("Insert"):
      scancode = 0xE052;
      break;
    case Hash("Home"):
      scancode = 0xE047;
      break;
    case Hash("PageUp"):
      scancode = 0xE049;
      break;
    case Hash("PageDown"):
      scancode = 0xE051;
      break;
    case Hash("Delete"):
      scancode = 0xE053;
      break;
    case Hash("End"):
      scancode = 0xE04F;
      break;
    case Hash("ArrowUp"):
      scancode = 0xE048;
      break;
    case Hash("ArrowDown"):
      scancode = 0xE050;
      break;
    case Hash("ArrowRight"):
      scancode = 0xE04D;
      break;
    case Hash("ArrowLeft"):
      scancode = 0xE04B;
      break;
    case Hash("NumLock"):
      scancode = 0xE045;
      break;
    case Hash("NumpadDivide"):
      scancode = 0xE035;
      break;
    case Hash("NumpadMultiply"):
      scancode = 0x0037;
      break;
    case Hash("NumpadSubtract"):
      scancode = 0x004A;
      break;
    case Hash("NumpadAdd"):
      scancode = 0x004E;
      break;
    case Hash("NumpadEnter"):
      scancode = 0xE01C;
      break;
    case Hash("Numpad1"):
      scancode = 0x004F;
      break;
    case Hash("Numpad2"):
      scancode = 0x0050;
      break;
    case Hash("Numpad3"):
      scancode = 0x0051;
      break;
    case Hash("Numpad4"):
      scancode = 0x004B;
      break;
    case Hash("Numpad5"):
      scancode = 0x004C;
      break;
    case Hash("Numpad6"):
      scancode = 0x004D;
      break;
    case Hash("Numpad7"):
      scancode = 0x0047;
      break;
    case Hash("Numpad8"):
      scancode = 0x0048;
      break;
    case Hash("Numpad9"):
      scancode = 0x0049;
      break;
    case Hash("Numpad0"):
      scancode = 0x0052;
      break;
    case Hash("NumpadDecimal"):
      scancode = 0x0053;
      break;
    case Hash("IntlBackslash"):
      scancode = 0x0056;
      break;
    case Hash("ContextMenu"):
      scancode = 0xE05D;
      break;
    case Hash("NumpadEqual"):
      scancode = 0x0059;
      break;
    case Hash("Power"):
      scancode = 0xE05E;
      break;
    case Hash("Help"):
      scancode = 0xE03B;
      break;
    case Hash("Undo"):
      scancode = 0xE008;
      break;
    case Hash("Cut"):
      scancode = 0xE017;
      break;
    case Hash("Copy"):
      scancode = 0xE018;
      break;
    case Hash("Paste"):
      scancode = 0xE00A;
      break;
    case Hash("AudioVolumeMute"):
      scancode = 0xE020;
      break;
    case Hash("AudioVolumeUp"):
      scancode = 0xE030;
      break;
    case Hash("AudioVolumeDown"):
      scancode = 0xE02E;
      break;
    case Hash("NumpadComma"):
      scancode = 0x007E;
      break;
    case Hash("IntlRo"):
      scancode = 0x0073;
      break;
    case Hash("KanaMode"):
      scancode = 0x0070;
      break;
    case Hash("IntlYen"):
      scancode = 0x007D;
      break;
    case Hash("Convert"):
      scancode = 0x0079;
      break;
    case Hash("NonConvert"):
      scancode = 0x007B;
      break;
    case Hash("Lang1"):
      scancode = 0x0072;
      break;
    case Hash("Lang2"):
      scancode = 0x0071;
      break;
    case Hash("Lang3"):
      scancode = 0x0078;
      break;
    case Hash("Lang4"):
      scancode = 0x0077;
      break;
    case Hash("MediaTrackNext"):
      scancode = 0xE019;
      break;
    case Hash("MediaTrackPrevious"):
      scancode = 0xE010;
      break;
    case Hash("MediaStop"):
      scancode = 0xE024;
      break;
    case Hash("Eject"):
      scancode = 0xE02C;
      break;
    case Hash("MediaPlayPause"):
      scancode = 0xE022;
      break;
    case Hash("MediaSelect"):
      scancode = 0xE06D;
      break;
    case Hash("LaunchMail"):
      scancode = 0xE06C;
      break;
    case Hash("LaunchApp2"):
      scancode = 0xE021;
      break;
    case Hash("LaunchApp1"):
      scancode = 0xE06B;
      break;
    case Hash("BrowserSearch"):
      scancode = 0xE065;
      break;
    case Hash("BrowserHome"):
      scancode = 0xE032;
      break;
    case Hash("BrowserBack"):
      scancode = 0xE06A;
      break;
    case Hash("BrowserForward"):
      scancode = 0xE069;
      break;
    case Hash("BrowserStop"):
      scancode = 0xE068;
      break;
    case Hash("BrowserRefresh"):
      scancode = 0xE067;
      break;
    case Hash("BrowserFavorites"):
      scancode = 0xE066;
      break;
    case Hash("Sleep"):
      scancode = 0xE05F;
      break;
    case Hash("WakeUp"):
      scancode = 0xE063;
      break;
    default:
      scancode = UINT32_MAX;
      break;
  }

  auto layout = GetKeyboardLayout(0);
  auto vk = MapVirtualKeyEx(scancode, MAPVK_VSC_TO_VK, layout);

  return vk == 0 ? UINT32_MAX : vk;
}

UINT MapWinModifierKey(const std::string& key) {
  switch (Hash(key.c_str())) {
    case Hash("Control"):
      return MOD_CONTROL;
    case Hash("ControlRight"):
      return MOD_CONTROL;
    case Hash("ControlLeft"):
      return MOD_CONTROL;
    case Hash("Shift"):
      return MOD_SHIFT;
    case Hash("ShiftRight"):
      return MOD_SHIFT;
    case Hash("ShiftLeft"):
      return MOD_SHIFT;
    case Hash("Alt"):
      return MOD_ALT;
    case Hash("AltRight"):
      return MOD_ALT;
    case Hash("AltLeft"):
      return MOD_ALT;
    case Hash("Meta"):
      return MOD_WIN;
    case Hash("MetaRight"):
      return MOD_WIN;
    case Hash("MetaLeft"):
      return MOD_WIN;
    default:
      return UINT32_MAX;
  }
}

UINT ToWinKey(const std::vector<std::string>& keys) {
  for (auto key : keys) {
    auto vk = MapWinVirtualKey(key);

    if (vk != UINT32_MAX) {
      return vk;
    }
  }

  return UINT32_MAX;
}

UINT ToWinModifiers(const std::vector<std::string>& keys) {
  UINT modifier = 0;
  for (auto key : keys) {
    auto candidate = MapWinModifierKey(key);

    if (candidate != UINT32_MAX) {
      modifier |= candidate;
    }
  }
  return modifier;
}

}  // namespace

namespace hotcakey {

Result Activate() {
  LOG("try to activate hotcakey");

  if (isActive.load(std::memory_order_acquire)) {
    LOG("already activated");
    return Result::kSuccess;
  }

  {
    std::unique_lock<std::mutex> lock(mutex);

    nativeThread = std::thread([] {
      LOG("native thread started");

      {
        std::lock_guard<std::mutex> lock(mutex);
        isActive.store(true, std::memory_order_release);
      }

      cond.notify_one();

      LOG("start message loop");

      MSG msg;

      while (isActive.load(std::memory_order_acquire)) {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) != 0) {
          auto id = (int)msg.wParam;
          auto modifiers = (UINT)LOWORD(msg.lParam);
          auto key = (UINT)HIWORD(msg.lParam);

          switch (msg.message) {
            case WM_SETHOTKEY: {
              // register
              if (key != 0 && modifiers != 0) {
                auto ok = RegisterHotKey(NULL, id, modifiers, key);
                if (!ok) {
                  ERR("failed to register hotkey");
                  // TODO: handle error
                }
                LOG("successfully register listener with id: " << id);
                // unregister
              } else {
                auto ok = UnregisterHotKey(NULL, id);
                if (!ok) {
                  ERR("failed to unregister hotkey");
                  // TODO: handle error
                }
                LOG("successfully unregister listener with id: " << id);
              }
              break;
            }
            case WM_HOTKEY: {
              // notify keydown event
              {
                std::lock_guard<std::mutex> lock(mutex);
                auto listener = listeners.at(id);
                listener->callback(hotcakey::Event(
                    hotcakey::EventType::kKeyDown, std::time(nullptr)));
              }  // lock(mutex)

              // observe keyup event
              auto observer = std::thread([key] {
                while ((GetAsyncKeyState(key) & (1 << 15)) != 0) {
                  std::this_thread::sleep_for(std::chrono::microseconds(50));
                }
              });
              observer.join();

              // notify keyup event
              {
                std::lock_guard<std::mutex> lock(mutex);
                auto listener = listeners.at(id);
                listener->callback(hotcakey::Event(hotcakey::EventType::kKeyUp,
                                                   std::time(nullptr)));
              }  // lock(mutex)

              break;
            }
            case WM_QUIT:
              break;
          }
        }
      }

      LOG("message loop stopped");
    });

    cond.wait(lock, [] { return isActive.load(std::memory_order_acquire); });
  }  // lock(mutex)

  LOG("message loop thread successfully started");

  return Result::kSuccess;
}

Result Inactivate() {
  LOG("deactivate hotcakey");

  if (!isActive.load(std::memory_order_acquire)) {
    LOG("do nothing since already inactive");
    return kSuccess;
  }

  LOG("unregister all event listeners");

  auto tid = GetThreadId(nativeThread.native_handle());

  {
    std::lock_guard<std::mutex> lock(mutex);

    for (auto [key, value] : listeners) {
      auto ok = PostThreadMessage(tid, WM_SETHOTKEY, key, 0);

      if (!ok) {
        ERR("failed to post thread message: " << GetLastError())
      }

      delete value;
    }

    listeners.clear();
  }  // lock(mutex)

  isActive.store(false, std::memory_order_release);

  auto ok = PostThreadMessage(tid, WM_QUIT, 0, 0);

  if (!ok) {
    ERR("failed to post thread message: " << GetLastError())
  }

  LOG("try to join event target thread");

  nativeThread.join();

  LOG("successfully shutdown");

  return kSuccess;
}

RegistrationResult Register(
    const std::vector<std::string>& keys,
    const std::function<void(hotcakey::Event)>& listener) {
  LOG("register hotkey");

  UINT key = ToWinKey(keys);
  UINT modifier = ToWinModifiers(keys) + MOD_NOREPEAT;  // ToWinModifiers(keys);

  LOG("key: " << key);
  LOG("modifier: " << modifier);

  auto id = ++eventHotKeyIdSequence;

  auto tid = GetThreadId(nativeThread.native_handle());

  // NOTICE: small hack!
  // we use the WM_SETHOTKEY message as setting or removing global hotkey event
  // apart from [its original
  // usage](https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-sethotkey)
  auto ok = PostThreadMessage(tid, WM_SETHOTKEY, id, MAKELONG(modifier, key));

  if (!ok) {
    ERR("failed to post thread message: " << GetLastError())
    return {kFailure, -1};
  }

  {
    std::lock_guard<std::mutex> lock(mutex);

    listeners[id] = new Listener{
        id,
        listener,
    };
  }  // lock(mutex)

  LOG("hotkey registered with id: " << id);

  return {kSuccess, id};
}

Result Unregister(const Registration& registration) {
  if (listeners.count(registration) == 0) {
    return kSuccess;
  }

  auto listener = listeners.at(registration);
  auto tid = GetThreadId(nativeThread.native_handle());
  auto ok = PostThreadMessage(tid, WM_SETHOTKEY, listener->registration, 0);

  if (!ok) {
    ERR("failed to post thread message: " << GetLastError())
    return kFailure;
  }

  {
    std::unique_lock<std::mutex> lock(mutex);
    listeners.erase(registration);
  }  // lock(mutex)

  delete listener;

  LOG("hotkey unregistered");

  return kSuccess;
}

}  // namespace hotcakey
