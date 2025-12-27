#pragma once
#include "functional"
#include "vector"

template <typename... Args>
class Event {
 public:
  using Handler = std::function<void(Args...)>;
  void Subscribe(Handler handler) {
    handlers_.push_back(handler);
  }
  void Invoke(Args... args) {
    for (auto& h : handlers_)
      h(args...);
  }

 private:
  std::vector<Handler> handlers_;
};
