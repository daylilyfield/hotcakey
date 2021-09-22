#include <carbon/carbon.h>
#include <pthread.h>

static std::thread nativeThread;
static std::atomic<bool> isActive(false);

// static CFRunLoopRef dispatcherLoopRef;

namespace HotCakey {

OSStatus HotkeyPressEventHandler(EventHandlerCallRef nextHandler, EventRef event, void* data) {
    LOG("hotkey pressed");
	// Q_UNUSED(nextHandler);
	// Q_UNUSED(data);

	if (GetEventClass(event) == kEventClassKeyboard &&
		GetEventKind(event) == kEventHotKeyPressed) {
		EventHotKeyID hkeyID;
		GetEventParameter(event,
						  kEventParamDirectObject,
						  typeEventHotKeyID,
						  NULL,
						  sizeof(EventHotKeyID),
						  NULL,
						  &hkeyID);
		// hotkeyPrivate->activateShortcut({hkeyID.signature, hkeyID.id});
    LOG("Press");
	}

	return noErr;
}
OSStatus HotkeyReleaseEventHandler(EventHandlerCallRef nextHandler, EventRef event, void* data) {
    LOG("hotkey relased");
	// Q_UNUSED(nextHandler);
	// Q_UNUSED(data);

	if (GetEventClass(event) == kEventClassKeyboard &&
		GetEventKind(event) == kEventHotKeyReleased) {
		EventHotKeyID hkeyID;
		GetEventParameter(event,
											kEventParamDirectObject,
											typeEventHotKeyID,
											NULL,
											sizeof(EventHotKeyID),
											NULL,
											&hkeyID);
		// hotkeyPrivate->releaseShortcut({hkeyID.signature, hkeyID.id});
    LOG("Release");
	}

	return noErr;
}

  void Deactivate() {
    LOG("deactivate hotcakey");

    isActive.store(false, std::memory_order_release);
  }

  void Activate() {
    LOG("activate hotcakey");

    isActive.store(true, std::memory_order_release);

    nativeThread = std::thread([] {

      EventTypeSpec pressEventSpec;
      pressEventSpec.eventClass = kEventClassKeyboard;
      pressEventSpec.eventKind = kEventHotKeyPressed;

      auto status = InstallApplicationEventHandler(NewEventHandlerUPP(HotCakey::HotkeyPressEventHandler), 1, &pressEventSpec, NULL, NULL);

      LOG("application event handler installed with status:" << status);
      
      EventTypeSpec releaseEventSpec;
      releaseEventSpec.eventClass = kEventClassKeyboard;
      releaseEventSpec.eventKind = kEventHotKeyReleased;

      InstallApplicationEventHandler(NewEventHandlerUPP(HotCakey::HotkeyReleaseEventHandler), 1, &releaseEventSpec, NULL, NULL);

      LOG("application event handler installed");

      // id autoReleasePool;
      // Class NSAutoreleasePool_class = (Class) objc_getClass("NSAutoreleasePool");

      LOG("start message loop");

      // dispatcherLoopRef = CFRunLoopGetCurrent();

     	// id pool = class_createInstance(NSAutoreleasePool_class, 0);
     	// autoReleasePool = ((id(*)(id, SEL))objc_msgSend)(pool, sel_registerName("init"));

      // CFRunLoopRun();
      // RunApplicationEventLoop();
      EventTargetRef target = GetEventDispatcherTarget();
          while(isActive.load(std::memory_order_acquire)) {
              EventRef event;	  
      		if (ReceiveNextEvent(0, NULL, (kEventDurationSecond / 10), true, &event) == noErr) {
      			SendEventToEventTarget (event, target);
      			ReleaseEvent(event);
      		}
              pthread_testcancel();
          }

      // ((id(*)(id, SEL))objc_msgSend)(autoReleasePool, sel_registerName("release"));


      LOG("message loop stopped");

    });

    pthread_attr_t hook_thread_attr;
    pthread_attr_init(&hook_thread_attr);

    // Get the policy and priority for the thread attr.
    int policy;
    pthread_attr_getschedpolicy(&hook_thread_attr, &policy);
    int priority = sched_get_priority_max(policy);
    struct sched_param param;
    param.sched_priority = sched_get_priority_max(policy);
    if (pthread_setschedparam(nativeThread.native_handle(), SCHED_OTHER, &param) != 0)
      std::cout << "Priority set Error" << std::endl;
  }

  void Register() {
    LOG("register hotkey");

  
    EventHotKeyID hkeyID;
    hkeyID.signature = 49; // SPACE
    hkeyID.id = 49;
  
  	EventHotKeyRef eventRef = 0;
  	OSStatus status = RegisterEventHotKey(
                        49,
  										  optionKey,
  										  hkeyID,
  										  // GetEventDispatcherTarget(),
                        GetApplicationEventTarget(),
  										  0,
  										  &eventRef);
  
  	if (status != noErr) {
      LOG("Error");
  		return;
  	} else {
      LOG("Registered");
  		return;
  	}
  }
}
