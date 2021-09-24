#include <cstddef>
#include <thread>
#include <chrono>
#include <napi.h>

#include "./hotcakey/hotcakey.h"
#include "./hotcakey/utils/strings.h"
#include "./hotcakey/utils/logger.h"

namespace {

class ActivationWorker : public Napi::AsyncWorker {

public:
  ActivationWorker(const Napi::Env& env, const Napi::Promise::Deferred& deferred)
  :Napi::AsyncWorker(env), deferred(deferred) {
  }

  void Execute () {
    result = hotcakey::Activate();
  }

  void OnError(const Napi::Error& e) {
     Napi::HandleScope scope(Env());
     deferred.Reject(Napi::String::New(Env(), "failure"));
  }

  void OnOK() {
    Napi::HandleScope scope(Env());

    LOG("activation callback called");

    switch (result) {
    case hotcakey::Result::kSuccess:
      LOG("activation finished with status: success");
      deferred.Resolve(Napi::String::New(Env(), hotcakey::ToString(result)));
      break;
    default:
      LOG("activation finished with status: failure");
      deferred.Reject(Napi::String::New(Env(), hotcakey::ToString(result)));
      break;
    }
  }
    
private:
  Napi::Promise::Deferred deferred;
  hotcakey::Result result;
};

std::vector<std::string> NormalizeKeys(const Napi::Array& keys) {
  auto results = std::vector<std::string>();

  for (uint32_t i = 0; i < keys.Length(); i++) {
    Napi::Value value = keys[i];
    auto key = value.As<Napi::String>().Utf8Value();
    results.push_back(hotcakey::utils::ToLower(key));
  }

  LOG("normalized key strings: " << hotcakey::utils::Join(results, ", "));

  return results;
}

void Unregister(const Napi::CallbackInfo& info) {
  LOG("start exported function `Unregister`");

  auto registration = static_cast<hotcakey::Registration*>(info.Data());

  if (registration != nullptr) {
    LOG("valid registration accepted");
    auto result = hotcakey::Unregister(*registration);

    if (result != hotcakey::Result::kSuccess) {
      Napi::TypeError::New(info.Env(), "cannot unregister listener").ThrowAsJavaScriptException();
      return;
    }

    delete registration;
  }
}

Napi::Value Register(const Napi::CallbackInfo& info) {
  LOG("start exported function `Register`");

  auto env = info.Env();

  if (info.Length() < 2 || !info[0].IsArray() || !info[1].IsFunction()) {
    Napi::TypeError::New(env, "invalid arguments").ThrowAsJavaScriptException();
    return env.Undefined();
  }

  auto keys = info[0].As<Napi::Array>();
  auto callback = info[1].As<Napi::Function>();

  auto listener = Napi::ThreadSafeFunction::New(env, callback, "HotCakey Listener", 0, 1);

  auto [result, registration] = hotcakey::Register(NormalizeKeys(keys), [listener](const hotcakey::Event& event){
    auto wrapper = [](Napi::Env env, Napi::Function jsCallback, hotcakey::Event* value) {
      LOG("call wrapper from thread safe function");

      auto event = Napi::Object::New(env);
      event["type"] = Napi::String::New(env, hotcakey::ToString(value->type));
      event["time"] = Napi::Number::New(env, value->time);

      jsCallback.Call( { event } );

      delete value;
    };

    LOG("callback " << event.type << " at " << event.time);

    auto value = new hotcakey::Event(event.type, event.time);
    auto status = listener.BlockingCall(value, wrapper);

    if (status != napi_ok) {
      LOG("failed to invoke thread safe function");
    }
  });

  if (result == hotcakey::Result::kSuccess) {
    auto data = new hotcakey::Registration(registration);
    return Napi::Function::New(env, Unregister, "Unregister", data);
  }

  return env.Undefined();
}

Napi::Promise Activate(const Napi::CallbackInfo& info) {
  LOG("start exported function `Activate`");

  auto env = info.Env();
  auto deferred = Napi::Promise::Deferred::New(info.Env());

  auto worker = new ActivationWorker(env, deferred);
  worker->Queue();

  return deferred.Promise();
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  LOG("initialize napi export object");

  exports["activate"] = Napi::Function::New(env, Activate);
  // exports["inactivate"] = Napi::Function::New(env, Activate);
  exports["register"] = Napi::Function::New(env, Register);

  return exports;
}

NODE_API_MODULE(hotcakey, Init);

} // namespace
