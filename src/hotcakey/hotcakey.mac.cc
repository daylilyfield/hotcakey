#include "./hotcakey.h"

#include <MacTypes.h>
#include <carbon/carbon.h>
#include <pthread.h>

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <ctime>
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
  EventHotKeyRef eventRef;
};

std::thread nativeThread;

// why do we use `atomic<bool> instead of `bool with mutex`?
// because this variable is read in the Carbon Event Loop,
// we should not repeat to lock and release mutext for performance reason.
std::atomic<bool> isActive(false);

std::unordered_map<hotcakey::Registration, const Listener*> listeners;

std::mutex mutex;
std::condition_variable cond;

hotcakey::Registration eventHotKeyIdSequence = 0;

OSStatus HandleKeyEvent(EventHandlerCallRef nextHandler, EventRef event,
                        void* data) {
  LOG("try to handle key event");

  auto clazz = GetEventClass(event);

  if (clazz != kEventClassKeyboard) {
    WRN("not a keyboard event: " << clazz);
    return noErr;
  }

  auto kind = GetEventKind(event);

  if (kind != kEventHotKeyPressed && kind != kEventHotKeyReleased) {
    WRN("not a key pressed or key released event: " << kind);
    return noErr;
  }

  EventHotKeyID eventHotKeyId;
  GetEventParameter(event, kEventParamDirectObject, typeEventHotKeyID, NULL,
                    sizeof(EventHotKeyID), NULL, &eventHotKeyId);

  auto listener = listeners.at(eventHotKeyId.id);

  switch (GetEventKind(event)) {
    case kEventHotKeyPressed:
      LOG("callback listener with keydown");
      listener->callback(
          hotcakey::Event(hotcakey::EventType::kKeyDown, std::time(nullptr)));
      break;
    case kEventHotKeyReleased:
      LOG("callback listener with keyup");
      listener->callback(
          hotcakey::Event(hotcakey::EventType::kKeyUp, std::time(nullptr)));
      break;
  }

  return noErr;
}

OSStatus InstallKeyEventHandler() {
  auto handler = NewEventHandlerUPP(HandleKeyEvent);

  EventTypeSpec spec[2];
  spec[0].eventClass = kEventClassKeyboard;
  spec[0].eventKind = kEventHotKeyPressed;
  spec[1].eventClass = kEventClassKeyboard;
  spec[1].eventKind = kEventHotKeyReleased;

  return InstallApplicationEventHandler(handler, 2, spec, NULL, NULL);
}

constexpr unsigned long long int Hash(const char* str,
                                      unsigned long long int hash = 0) {
  return (*str == 0) ? hash : 101 * Hash(str + 1) + *str;
}

UInt32 MapCarbonVirtualKey(const std::string& key) {
  switch (Hash(key.c_str())) {
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
      return kVK_ANSI_A;
    case Hash("KeyB"):
      return kVK_ANSI_B;
    case Hash("KeyC"):
      return kVK_ANSI_C;
    case Hash("KeyD"):
      return kVK_ANSI_D;
    case Hash("KeyE"):
      return kVK_ANSI_E;
    case Hash("KeyF"):
      return kVK_ANSI_F;
    case Hash("KeyG"):
      return kVK_ANSI_G;
    case Hash("KeyH"):
      return kVK_ANSI_H;
    case Hash("KeyI"):
      return kVK_ANSI_I;
    case Hash("KeyJ"):
      return kVK_ANSI_J;
    case Hash("KeyK"):
      return kVK_ANSI_K;
    case Hash("KeyL"):
      return kVK_ANSI_L;
    case Hash("KeyM"):
      return kVK_ANSI_M;
    case Hash("KeyN"):
      return kVK_ANSI_N;
    case Hash("KeyO"):
      return kVK_ANSI_O;
    case Hash("KeyP"):
      return kVK_ANSI_P;
    case Hash("KeyQ"):
      return kVK_ANSI_Q;
    case Hash("KeyR"):
      return kVK_ANSI_R;
    case Hash("KeyS"):
      return kVK_ANSI_S;
    case Hash("KeyT"):
      return kVK_ANSI_T;
    case Hash("KeyU"):
      return kVK_ANSI_U;
    case Hash("KeyV"):
      return kVK_ANSI_V;
    case Hash("KeyW"):
      return kVK_ANSI_W;
    case Hash("KeyX"):
      return kVK_ANSI_X;
    case Hash("KeyY"):
      return kVK_ANSI_Y;
    case Hash("KeyZ"):
      return kVK_ANSI_Z;
    case Hash("Digit1"):
      return kVK_ANSI_1;
    case Hash("Digit2"):
      return kVK_ANSI_2;
    case Hash("Digit3"):
      return kVK_ANSI_3;
    case Hash("Digit4"):
      return kVK_ANSI_4;
    case Hash("Digit5"):
      return kVK_ANSI_5;
    case Hash("Digit6"):
      return kVK_ANSI_6;
    case Hash("Digit7"):
      return kVK_ANSI_7;
    case Hash("Digit8"):
      return kVK_ANSI_8;
    case Hash("Digit9"):
      return kVK_ANSI_9;
    case Hash("Digit0"):
      return kVK_ANSI_0;
    case Hash("Minus"):
      return kVK_ANSI_Minus;
    case Hash("Equal"):
      return kVK_ANSI_Equal;
    case Hash("BracketLeft"):
      return kVK_ANSI_LeftBracket;
    case Hash("BracketRight"):
      return kVK_ANSI_RightBracket;
    case Hash("Backslash"):
      return kVK_ANSI_Backslash;
    case Hash("Semicolon"):
      return kVK_ANSI_Semicolon;
    case Hash("Quote"):
      return kVK_ANSI_Quote;
    case Hash("Backquote"):
      return kVK_ANSI_Grave;
    case Hash("Comma"):
      return kVK_ANSI_Comma;
    case Hash("Period"):
      return kVK_ANSI_Period;
    case Hash("Slash"):
      return kVK_ANSI_Slash;
    case Hash("Enter"):
      return kVK_Return;
    case Hash("Escape"):
      return kVK_Escape;
    case Hash("Backspace"):
      return kVK_Delete;
    case Hash("Tab"):
      return kVK_Tab;
    case Hash("Space"):
      return kVK_Space;
    case Hash("CapsLock"):
      return kVK_CapsLock;
    case Hash("F1"):
      return kVK_F1;
    case Hash("F2"):
      return kVK_F2;
    case Hash("F3"):
      return kVK_F3;
    case Hash("F4"):
      return kVK_F4;
    case Hash("F5"):
      return kVK_F5;
    case Hash("F6"):
      return kVK_F6;
    case Hash("F7"):
      return kVK_F7;
    case Hash("F8"):
      return kVK_F8;
    case Hash("F9"):
      return kVK_F9;
    case Hash("F10"):
      return kVK_F10;
    case Hash("F11"):
      return kVK_F11;
    case Hash("F12"):
      return kVK_F12;
    case Hash("F13"):
      return kVK_F13;
    case Hash("F14"):
      return kVK_F14;
    case Hash("F15"):
      return kVK_F15;
    case Hash("F16"):
      return kVK_F16;
    case Hash("F17"):
      return kVK_F17;
    case Hash("F18"):
      return kVK_F18;
    case Hash("F19"):
      return kVK_F19;
    case Hash("F20"):
      return kVK_F20;
    case Hash("F21"):
      return UINT32_MAX;
    case Hash("F22"):
      return UINT32_MAX;
    case Hash("F23"):
      return UINT32_MAX;
    case Hash("F24"):
      return UINT32_MAX;
    case Hash("PrintScreen"):
      return UINT32_MAX;
    case Hash("ScrollLock"):
      return UINT32_MAX;
    case Hash("Pause"):
      return UINT32_MAX;
    case Hash("Insert"):
      return kVK_Help;
    case Hash("Home"):
      return kVK_Home;
    case Hash("PageUp"):
      return kVK_PageUp;
    case Hash("PageDown"):
      return kVK_PageDown;
    case Hash("Delete"):
      return kVK_ForwardDelete;
    case Hash("End"):
      return kVK_End;
    case Hash("ArrowUp"):
      return kVK_UpArrow;
    case Hash("ArrowDown"):
      return kVK_DownArrow;
    case Hash("ArrowRight"):
      return kVK_RightArrow;
    case Hash("ArrowLeft"):
      return kVK_LeftArrow;
    case Hash("NumLock"):
      return kVK_ANSI_KeypadClear;
    case Hash("NumpadDivide"):
      return kVK_ANSI_KeypadDivide;
    case Hash("NumpadMultiply"):
      return kVK_ANSI_KeypadMultiply;
    case Hash("NumpadSubtract"):
      return kVK_ANSI_KeypadMinus;
    case Hash("NumpadAdd"):
      return kVK_ANSI_KeypadPlus;
    case Hash("NumpadEnter"):
      return kVK_ANSI_KeypadEnter;
    case Hash("Numpad1"):
      return kVK_ANSI_Keypad1;
    case Hash("Numpad2"):
      return kVK_ANSI_Keypad2;
    case Hash("Numpad3"):
      return kVK_ANSI_Keypad3;
    case Hash("Numpad4"):
      return kVK_ANSI_Keypad4;
    case Hash("Numpad5"):
      return kVK_ANSI_Keypad5;
    case Hash("Numpad6"):
      return kVK_ANSI_Keypad6;
    case Hash("Numpad7"):
      return kVK_ANSI_Keypad7;
    case Hash("Numpad8"):
      return kVK_ANSI_Keypad8;
    case Hash("Numpad9"):
      return kVK_ANSI_Keypad9;
    case Hash("Numpad0"):
      return kVK_ANSI_Keypad0;
    case Hash("NumpadDecimal"):
      return kVK_ANSI_KeypadDecimal;
    case Hash("IntlBackslash"):
      return kVK_ISO_Section;
    case Hash("ContextMenu"):
      return UINT32_MAX;
    case Hash("NumpadEqual"):
      return kVK_ANSI_KeypadEquals;
    case Hash("Power"):
      return UINT32_MAX;
    case Hash("Help"):
      return kVK_Help;
    case Hash("Undo"):
      return UINT32_MAX;
    case Hash("Cut"):
      return UINT32_MAX;
    case Hash("Copy"):
      return UINT32_MAX;
    case Hash("Paste"):
      return UINT32_MAX;
    case Hash("AudioVolumeMute"):
      return kVK_Mute;
    case Hash("AudioVolumeUp"):
      return kVK_VolumeUp;
    case Hash("AudioVolumeDown"):
      return kVK_VolumeDown;
    case Hash("NumpadComma"):
      return kVK_JIS_KeypadComma;
    case Hash("IntlRo"):
      return kVK_JIS_Underscore;
    case Hash("KanaMode"):
      return kVK_JIS_Kana;
    case Hash("IntlYen"):
      return kVK_JIS_Yen;
    case Hash("Convert"):
      return UINT32_MAX;
    case Hash("NonConvert"):
      return UINT32_MAX;
    case Hash("Lang1"):
      return kVK_JIS_Kana;
    case Hash("Lang2"):
      return kVK_JIS_Eisu;
    case Hash("Lang3"):
      return UINT32_MAX;
    case Hash("Lang4"):
      return UINT32_MAX;
    case Hash("MediaTrackNext"):
      return UINT32_MAX;
    case Hash("MediaTrackPrevious"):
      return UINT32_MAX;
    case Hash("MediaStop"):
      return UINT32_MAX;
    case Hash("Eject"):
      return UINT32_MAX;
    case Hash("MediaPlayPause"):
      return UINT32_MAX;
    case Hash("MediaSelect"):
      return UINT32_MAX;
    case Hash("LaunchMail"):
      return UINT32_MAX;
    case Hash("LaunchApp2"):
      return UINT32_MAX;
    case Hash("LaunchApp1"):
      return UINT32_MAX;
    case Hash("BrowserSearch"):
      return UINT32_MAX;
    case Hash("BrowserHome"):
      return UINT32_MAX;
    case Hash("BrowserBack"):
      return UINT32_MAX;
    case Hash("BrowserForward"):
      return UINT32_MAX;
    case Hash("BrowserStop"):
      return UINT32_MAX;
    case Hash("BrowserRefresh"):
      return UINT32_MAX;
    case Hash("BrowserFavorites"):
      return UINT32_MAX;
    case Hash("Sleep"):
      return UINT32_MAX;
    case Hash("WakeUp"):
      return UINT32_MAX;
    default:
      return UINT32_MAX;
  }
}

UInt32 MapCarbonModifierKey(const std::string& key) {
  switch (Hash(key.c_str())) {
    case Hash("Control"):
      return controlKey;
    case Hash("ControlRight"):
      return controlKey;
    case Hash("ControlLeft"):
      return controlKey;
    case Hash("Shift"):
      return shiftKey;
    case Hash("ShiftRight"):
      return shiftKey;
    case Hash("ShiftLeft"):
      return shiftKey;
    case Hash("Alt"):
      return optionKey;
    case Hash("AltRight"):
      return optionKey;
    case Hash("AltLeft"):
      return optionKey;
    case Hash("Meta"):
      return cmdKey;
    case Hash("MetaRight"):
      return cmdKey;
    case Hash("MetaLeft"):
      return cmdKey;
    default:
      return UINT32_MAX;
  }
}

UInt32 ToCarbonKey(const std::vector<std::string>& keys) {
  for (auto key : keys) {
    auto vk = MapCarbonVirtualKey(key);

    if (vk != UINT32_MAX) {
      return vk;
    }
  }

  return UINT32_MAX;
}

UInt32 ToCarbonModifiers(const std::vector<std::string>& keys) {
  UInt32 modifier = 0;
  for (auto key : keys) {
    auto candidate = MapCarbonModifierKey(key);

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

      auto status = InstallKeyEventHandler();

      if (status != noErr) {
        ERR("application event handler installation failed with status:"
            << status);
        return;
      }

      LOG("application event handler installed");

      {
        std::lock_guard<std::mutex> lock(mutex);
        isActive.store(true, std::memory_order_release);
      }

      cond.notify_one();

      LOG("start message loop");

      EventTargetRef target = GetApplicationEventTarget();
      while (isActive.load(std::memory_order_acquire)) {
        EventRef event;
        if (ReceiveNextEvent(0, NULL, (kEventDurationSecond / 10), true,
                             &event) == noErr) {
          SendEventToEventTarget(event, target);
          ReleaseEvent(event);
        }
        pthread_testcancel();
      }

      LOG("message loop stopped");
    });

    pthread_attr_t hook_thread_attr;
    pthread_attr_init(&hook_thread_attr);

    int policy;
    pthread_attr_getschedpolicy(&hook_thread_attr, &policy);

    struct sched_param param;
    param.sched_priority = sched_get_priority_max(policy);

    if (pthread_setschedparam(nativeThread.native_handle(), SCHED_OTHER,
                              &param) != 0) {
      WRN("failed to set priority");
    }

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

  {
    std::lock_guard<std::mutex> lock(mutex);

    for (auto [key, value] : listeners) {
      if (value->eventRef == nullptr) continue;
      auto status = UnregisterEventHotKey(value->eventRef);

      if (status != noErr) {
        ERR("failed to unregister listener with id: "
            << value->registration << " and status: " << status);
        continue;
      }

      delete value;

      LOG("successfully unregister listener with id: " << value->registration);
    }

    listeners.clear();
  }  // lock(mutex)

  isActive.store(false, std::memory_order_release);

  LOG("try to join event target thread");

  nativeThread.join();

  LOG("successfully shutdown");

  return kSuccess;
}

RegistrationResult Register(
    const std::vector<std::string>& keys,
    const std::function<void(hotcakey::Event)>& listener) {
  LOG("register hotkey");

  auto key = ToCarbonKey(keys);
  auto modifier = ToCarbonModifiers(keys);

  if (key == UINT32_MAX) {
    ERR("cannot find a virtual key on current keyboard layout");
    return {kFailure, -1};
  }

  if (modifier == UINT32_MAX) {
    ERR("cannot find modifier keys");
    return {kFailure, -1};
  }

  LOG("key: " << key);
  LOG("modifier: " << modifier);

  auto id = ++eventHotKeyIdSequence;

  EventHotKeyID hkeyID;
  hkeyID.id = id;
  hkeyID.signature = key * modifier;

  EventHotKeyRef eventRef = NULL;
  OSStatus status = RegisterEventHotKey(
      key, modifier, hkeyID, GetApplicationEventTarget(), 0, &eventRef);

  if (status != noErr) {
    // -9878 means conflict maybe
    ERR("failed to register hotkey: " << status);
    return {kFailure, -1};
  }

  {
    std::lock_guard<std::mutex> lock(mutex);

    listeners[id] = new Listener{
        .registration = id,
        .callback = listener,
        .eventRef = eventRef,
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
  auto status = UnregisterEventHotKey(listener->eventRef);

  if (status != noErr) {
    ERR("failed to unregister hotkey with status: " << status);
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
