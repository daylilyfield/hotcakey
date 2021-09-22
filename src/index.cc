#include <napi.h>
#include <thread>
#include <unordered_map>
#include <iostream>
#include <chrono>

#include "./logger.cc"
#include "./hotcakey.mac.cc"

#if defined(__APPLE__)
#include <CoreFoundation/CoreFoundation.h>
#endif

enum HotCakeyEventType {
  kKeyDown,
  kKeyUp
};

typedef struct _HotCakeyEventData {
  
} HotCakeyEventData;

typedef struct _HotCakeyEvent {
  HotCakeyEventType type;
  HotCakeyEventData data;
} HotCakeyEvent;

class Worker : public Napi::AsyncProgressQueueWorker<HotCakeyEvent> {

public:
  Worker(Napi::Env env, HotCakeyEventType type, Napi::Array keys, Napi::Function handler)
  : Napi::AsyncProgressQueueWorker<HotCakeyEvent>(env) {
    this->type = type;
    this->keys = keys;
    this->handler.Reset(handler, 1);
  }

  ~Worker() {}

  void Execute(const ExecutionProgress &progress) override {
    LOG("Execute");
    HotCakeyEvent data({ .type = HotCakeyEventType::kKeyDown });
    progress.Send(&data, 1);
    StartMessageLoop();
    // std::this_thread::sleep_for(std::chrono::minutes(3));
  }

  void OnOK() override {
    LOG("OnOK");
    // HandleScope scope(Env());
    // Callback().Call({String::New(Env(), echo)});
  }
  
  void OnError(const Napi::Error &e) override {
    LOG("OnError");
    // HandleScope scope(Env());
    // if (!this->errorCallback.IsEmpty()) {
    //   this->errorCallback.Call(Receiver().Value(), {String::New(Env(), e.Message())});
    // }
  }

  void OnProgress(const HotCakeyEvent *data, size_t count) override {
    LOG("OnProgress");
    handler.Call({});
    InstallHotkey();
  }

private:
  HotCakeyEventType type;
  Napi::Array keys;
  Napi::FunctionReference handler;
};

// TODO: clean up
std::vector<Worker*> workers;
std::unordered_map<std::string, HotCakeyEventType> name2type;

void Off(const Napi::CallbackInfo& info) {
  LOG("Off");

  Worker* worker = (Worker*) info.Data();
  LOG("found worker: " << worker);
}

Napi::Function On(const Napi::CallbackInfo& info) {
  LOG("On");

  Napi::Env env = info.Env();

  if (info.Length() < 3) {
    Napi::TypeError::New(env, "Invalid argument count").ThrowAsJavaScriptException();
    return Napi::Function::New(env, Off);
  }

  if (!info[0].IsString() || !info[2].IsFunction()) {
    Napi::TypeError::New(env, "Invalid argument types").ThrowAsJavaScriptException();
    return Napi::Function::New(env, Off);
  }

  Napi::String type = info[0].As<Napi::String>();
  Napi::Array keys = info[1].As<Napi::Array>();
  Napi::Function handler = info[2].As<Napi::Function>();

  Worker* worker = new Worker(env, name2type.at(type), keys, handler);
  worker->Queue();
  workers.push_back(worker);

  return Napi::Function::New(env, Off, "Off", worker);
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  LOG("Init");

  name2type["keydown"] = HotCakeyEventType::kKeyDown;
  name2type["keyup"] = HotCakeyEventType::kKeyUp;


  exports.Set(Napi::String::New(env, "on"), Napi::Function::New(env, On));
  return exports;
}

NODE_API_MODULE(hotcakey, Init)
