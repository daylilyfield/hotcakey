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

  switch (Hash(ch)) {
    // modifiers key to ignore
    case Hash("shift"):
      return UINT32_MAX;
    case Hash("option"):
      return UINT32_MAX;
    case Hash("alt"):
      return UINT32_MAX;
    case Hash("alt | option"):
      return UINT32_MAX;
    case Hash("control"):
      return UINT32_MAX;
    case Hash("ctrl"):
      return UINT32_MAX;
    case Hash("command"):
      return UINT32_MAX;
    case Hash("cmd"):
      return UINT32_MAX;
    case Hash("command | control"):
      return UINT32_MAX;
    case Hash("cmd | ctrl"):
      return UINT32_MAX;
    // 1 : 1 paired on any keyboard
    case Hash("f1"):
      return VK_F1;
    case Hash("f2"):
      return VK_F2;
    case Hash("f3"):
      return VK_F3;
    case Hash("f4"):
      return VK_F4;
    case Hash("f5"):
      return VK_F5;
    case Hash("f6"):
      return VK_F6;
    case Hash("f7"):
      return VK_F7;
    case Hash("f8"):
      return VK_F8;
    case Hash("f9"):
      return VK_F9;
    case Hash("f10"):
      return VK_F10;
    case Hash("f11"):
      return VK_F11;
    case Hash("f12"):
      return VK_F12;
    case Hash("f13"):
      return VK_F13;
    case Hash("f14"):
      return VK_F14;
    case Hash("f15"):
      return VK_F15;
    case Hash("f16"):
      return VK_F16;
    case Hash("f17"):
      return VK_F17;
    case Hash("f18"):
      return VK_F18;
    case Hash("f19"):
      return VK_F19;
    case Hash("f20"):
      return VK_F20;
    case Hash("f21"):
      return VK_F21;
    case Hash("f22"):
      return VK_F22;
    case Hash("f23"):
      return VK_F23;
    case Hash("f24"):
      return VK_F24;
    case Hash("space"):
      return VK_SPACE;
    case Hash("tab"):
      return VK_TAB;
    case Hash("capslock"):
      return VK_CAPITAL;
    case Hash("numlock"):
      return VK_NUMLOCK;
    case Hash("scrolllock"):
      return VK_SCROLL;
    case Hash("delete"):
      return VK_DELETE;
    case Hash("backspace"):
      return VK_BACK;
    case Hash("enter"):
      return VK_RETURN;
    case Hash("return"):
      return VK_RETURN;
    case Hash("arrowup"):
      return VK_UP;
    case Hash("arrowdown"):
      return VK_DOWN;
    case Hash("arrowleft"):
      return VK_LEFT;
    case Hash("arrowright"):
      return VK_RIGHT;
    case Hash("home"):
      return VK_HOME;
    case Hash("end"):
      return VK_END;
    case Hash("pageup"):
      return VK_PRIOR;
    case Hash("pagedown"):
      return VK_NEXT;
    case Hash("escape"):
      return VK_ESCAPE;
    case Hash("esc"):
      return VK_ESCAPE;
    case Hash("volumeup"):
      return VK_VOLUME_UP;
    case Hash("volumeedown"):
      return VK_VOLUME_DOWN;
    case Hash("volumemute"):
      return VK_VOLUME_MUTE;
    case Hash("medianext"):
      return VK_MEDIA_NEXT_TRACK;
    case Hash("mediaprev"):
      return VK_MEDIA_PREV_TRACK;
    case Hash("mediastop"):
      return VK_MEDIA_STOP;
    case Hash("mediaplaypause"):
      return VK_MEDIA_PLAY_PAUSE;
    case Hash("print"):
      return VK_PRINT;
    case Hash("num0"):
      return VK_NUMPAD0;
    case Hash("num1"):
      return VK_NUMPAD1;
    case Hash("num2"):
      return VK_NUMPAD2;
    case Hash("num3"):
      return VK_NUMPAD3;
    case Hash("num4"):
      return VK_NUMPAD4;
    case Hash("num5"):
      return VK_NUMPAD5;
    case Hash("num6"):
      return VK_NUMPAD6;
    case Hash("num7"):
      return VK_NUMPAD7;
    case Hash("num8"):
      return VK_NUMPAD8;
    case Hash("num9"):
      return VK_NUMPAD9;
    case Hash("numadd"):
      return VK_ADD;
    case Hash("numsub"):
      return VK_SUBTRACT;
    case Hash("nummult"):
      return VK_MULTIPLY;
    case Hash("numdiv"):
      return VK_DIVIDE;
    case Hash("numdec"):
      return VK_DECIMAL;
  }

  auto first = ch[0];
  if (first <= 0xFFFF) {
    auto vk = VkKeyScanW(static_cast<WCHAR>(first));
    return (vk > -1) ? LOBYTE(vk) : byte(first);
  }

  return UINT32_MAX;
}

UINT MapWinModifierKey(const std::string& key) {
  switch (Hash(key.c_str())) {
    case Hash("shift"):
      return MOD_SHIFT;
    case Hash("option"):
      return MOD_ALT;
    case Hash("alt"):
      return MOD_ALT;
    case Hash("alt | option"):
      return MOD_ALT;
    case Hash("control"):
      return MOD_CONTROL;
    case Hash("ctrl"):
      return MOD_CONTROL;
    case Hash("command"):
      return MOD_CONTROL;
    case Hash("cmd"):
      return MOD_CONTROL;
    case Hash("command | control"):
      return MOD_CONTROL;
    case Hash("cmd | ctrl"):
      return MOD_CONTROL;
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
