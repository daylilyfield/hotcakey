#include <thread>
#include <chrono>
#include <napi.h>

#include "./logger.cc"
#include "./hotcakey.cc"

// std::thread nativeThread;
Napi::ThreadSafeFunction listener;

void Off(const Napi::CallbackInfo& info) {
  LOG("start exported function `Off`");
}

Napi::Function On(const Napi::CallbackInfo& info) {
  LOG("start exported function `On`");

  auto env = info.Env();

  if (info.Length() < 3 || !info[0].IsString() || !info[1].IsArray() || !info[2].IsFunction()) {
    Napi::TypeError::New(env, "invalid arguments").ThrowAsJavaScriptException();
    return Napi::Function::New(env, Off);
  }

  auto type = info[0].As<Napi::String>();
  auto keys = info[1].As<Napi::Array>();
  auto callback = info[2].As<Napi::Function>();

  listener = Napi::ThreadSafeFunction::New(
      env,
      callback,
      "HotCakey Listener",
      0,             
      1,            
      []( Napi::Env ) {
        nativeThread.join();
      } );

  // nativeThread = std::thread([] {
    // HotCakey::Activate();

    // auto callback = []( Napi::Env env, Napi::Function jsCallback) {
    //   jsCallback.Call({});
    // };

    // listener.BlockingCall(callback);

    // std::this_thread::sleep_for( std::chrono::seconds( 1 ) );

    // listener.Release();
  // } );

  HotCakey::Register();

  return Napi::Function::New(env, Off, "Off", listener);
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  LOG("initialize napi export object");

    HotCakey::Activate();

  exports["on"] = Napi::Function::New(env, On);

  return exports;
}

NODE_API_MODULE(hotcakey, Init);
