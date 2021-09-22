#include <Carbon/Carbon.h>
#include <objc/objc.h>
#include <objc/objc-runtime.h>

#include "./logger.cc"

static id auto_release_pool;

OSStatus hotkeyPressEventHandler(EventHandlerCallRef nextHandler, EventRef event, void* data) {
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
OSStatus hotkeyReleaseEventHandler(EventHandlerCallRef nextHandler, EventRef event, void* data) {
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

void StartMessageLoop() {
  LOG("StartMessageLoop");

  Class NSAutoreleasePool_class = (Class) objc_getClass("NSAutoreleasePool");
									id pool = class_createInstance(NSAutoreleasePool_class, 0);
									auto_release_pool = ((id(*)(id, SEL))objc_msgSend)(pool, sel_registerName("init"));

  CFRunLoopRun();

  ((id(*)(id, SEL))objc_msgSend)(auto_release_pool, sel_registerName("release"));
}

void InstallHotkey() {
  LOG("install hotkey");


  EventTypeSpec pressEventSpec;
  pressEventSpec.eventClass = kEventClassKeyboard;
  pressEventSpec.eventKind = kEventHotKeyPressed;
  InstallApplicationEventHandler(hotkeyPressEventHandler, 1, &pressEventSpec, NULL, NULL);
  
  EventTypeSpec releaseEventSpec;
  releaseEventSpec.eventClass = kEventClassKeyboard;
  releaseEventSpec.eventKind = kEventHotKeyReleased;
  InstallApplicationEventHandler(hotkeyReleaseEventHandler, 1, &releaseEventSpec, NULL, NULL);

  LOG("application event handler installed");

  EventHotKeyID hkeyID;
  hkeyID.signature = 49; // SPACE
  hkeyID.id = 49;

	EventHotKeyRef eventRef = 0;
	OSStatus status = RegisterEventHotKey(
                      49,
										  optionKey,
										  hkeyID,
										  GetEventDispatcherTarget(),
                      // GetApplicationEventTarget(),
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

