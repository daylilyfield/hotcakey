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

CFMutableDictionaryRef charToCodeDict = NULL;

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
      return kVK_F1;
    case Hash("f2"):
      return kVK_F2;
    case Hash("f3"):
      return kVK_F3;
    case Hash("f4"):
      return kVK_F4;
    case Hash("f5"):
      return kVK_F5;
    case Hash("f6"):
      return kVK_F6;
    case Hash("f7"):
      return kVK_F7;
    case Hash("f8"):
      return kVK_F8;
    case Hash("f9"):
      return kVK_F9;
    case Hash("f10"):
      return kVK_F10;
    case Hash("f11"):
      return kVK_F11;
    case Hash("f12"):
      return kVK_F12;
    case Hash("f13"):
      return kVK_F13;
    case Hash("f14"):
      return kVK_F14;
    case Hash("f15"):
      return kVK_F15;
    case Hash("f16"):
      return kVK_F16;
    case Hash("f17"):
      return kVK_F17;
    case Hash("f18"):
      return kVK_F18;
    case Hash("f19"):
      return kVK_F19;
    case Hash("f20"):
      return kVK_F20;
    case Hash("space"):
      return kVK_Space;
    case Hash("tab"):
      return kVK_Tab;
    case Hash("capslock"):
      return kVK_CapsLock;
    case Hash("delete"):
      return kVK_ForwardDelete;
    case Hash("backspace"):
      return kVK_Delete;
    case Hash("enter"):
      return kVK_Return;
    case Hash("return"):
      return kVK_Return;
    case Hash("arrowup"):
      return kVK_UpArrow;
    case Hash("arrowdown"):
      return kVK_DownArrow;
    case Hash("arrowleft"):
      return kVK_LeftArrow;
    case Hash("arrowright"):
      return kVK_RightArrow;
    case Hash("home"):
      return kVK_Home;
    case Hash("end"):
      return kVK_End;
    case Hash("pageup"):
      return kVK_PageUp;
    case Hash("pagedown"):
      return kVK_PageDown;
    case Hash("escape"):
      return kVK_Escape;
    case Hash("esc"):
      return kVK_Escape;
    case Hash("volumeup"):
      return kVK_VolumeUp;
    case Hash("volumeedown"):
      return kVK_VolumeDown;
    case Hash("volumemute"):
      return kVK_Mute;
    // TODO MediaNextTrack, MediaPreviousTrack, MediaStop and MediaPlayPause
    // TODO PrintScreen
    case Hash("num0"):
      return kVK_ANSI_Keypad0;
    case Hash("num1"):
      return kVK_ANSI_Keypad1;
    case Hash("num2"):
      return kVK_ANSI_Keypad2;
    case Hash("num3"):
      return kVK_ANSI_Keypad3;
    case Hash("num4"):
      return kVK_ANSI_Keypad4;
    case Hash("num5"):
      return kVK_ANSI_Keypad5;
    case Hash("num6"):
      return kVK_ANSI_Keypad6;
    case Hash("num7"):
      return kVK_ANSI_Keypad7;
    case Hash("num8"):
      return kVK_ANSI_Keypad8;
    case Hash("num9"):
      return kVK_ANSI_Keypad9;
    case Hash("numadd"):
      return kVK_ANSI_KeypadPlus;
    case Hash("numsub"):
      return kVK_ANSI_KeypadMinus;
    case Hash("nummult"):
      return kVK_ANSI_KeypadMultiply;
    case Hash("numdiv"):
      return kVK_ANSI_KeypadDivide;
    case Hash("numdec"):
      return kVK_ANSI_KeypadDecimal;
  }

  // try to respect keyboard layout

  LOG("try to respect keyboard layout");

  // generate keycodes table (ref: https://stackoverflow.com/a/1971027)
  if (charToCodeDict == NULL) {
    size_t vk;
    charToCodeDict = CFDictionaryCreateMutable(
        kCFAllocatorDefault, 128, &kCFCopyStringDictionaryKeyCallBacks, NULL);

    if (charToCodeDict == NULL) {
      return UINT32_MAX;
    }

    TISInputSourceRef source = TISCopyCurrentKeyboardInputSource();
    CFDataRef uchr = static_cast<CFDataRef>(
        TISGetInputSourceProperty(source, kTISPropertyUnicodeKeyLayoutData));

    if (uchr == NULL) {
      // for japanese keyboard fallback
      source = TISCopyCurrentKeyboardLayoutInputSource();
      uchr = static_cast<CFDataRef>((
          TISGetInputSourceProperty(source, kTISPropertyUnicodeKeyLayoutData)));

      if (uchr == NULL) {
        LOG("cannnot detect layout data");
        return UINT32_MAX;
      }
    }

    CFRelease(source);

    const UCKeyboardLayout* layout =
        reinterpret_cast<const UCKeyboardLayout*>(CFDataGetBytePtr(uchr));

    for (vk = 0; vk < 128; ++vk) {
      UInt32 keysDown = 0;
      UniChar chars[4];
      UniCharCount realLength;

      UCKeyTranslate(layout, vk, kUCKeyActionDisplay, 0, LMGetKbdType(),
                     kUCKeyTranslateNoDeadKeysBit, &keysDown,
                     sizeof(chars) / sizeof(chars[0]), &realLength, chars);

      auto string = CFStringCreateWithCharacters(kCFAllocatorDefault, chars, 1);

      if (string != NULL) {
        CFDictionaryAddValue(charToCodeDict, string, (const void*)vk);
        CFRelease(string);
      }
    }
    LOG("keyboard layout table generated");
  }

  CGKeyCode code;
  UniChar character = key.c_str()[0];

  LOG("target character: " << character);

  auto charStrRef =
      CFStringCreateWithCharacters(kCFAllocatorDefault, &character, 1);

  if (!CFDictionaryGetValueIfPresent(charToCodeDict, charStrRef,
                                     (const void**)&code)) {
    LOG("cannot find key code mapping");
    return UINT32_MAX;
  }

  CFRelease(charStrRef);

  return code;
}

UInt32 MapCarbonModifierKey(const std::string& key) {
  switch (Hash(key.c_str())) {
    case Hash("shift"):
      return shiftKey;
    case Hash("option"):
      return optionKey;
    case Hash("alt"):
      return optionKey;
    case Hash("alt | option"):
      return optionKey;
    case Hash("control"):
      return controlKey;
    case Hash("ctrl"):
      return controlKey;
    case Hash("command"):
      return cmdKey;
    case Hash("cmd"):
      return cmdKey;
    case Hash("command | control"):
      return cmdKey;
    case Hash("cmd | ctrl"):
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

      EventTargetRef target = GetEventDispatcherTarget();
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

  LOG("unregistered hotkey");

  {
    std::unique_lock<std::mutex> lock(mutex);
    listeners.erase(registration);
  }  // lock(mutex)

  delete listener;

  return kSuccess;
}

}  // namespace hotcakey
