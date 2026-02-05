/**
 * @file EventBus.cpp
 * @brief Implementation of EventBus and EventHandle.
 */

#include "event_bus.hpp"

void EventHandle::Unsubscribe() {
  if (unsubscribed_) {
    return;
  }

  if (auto bus = bus_.lock()) {
    if (target_.has_value()) {
      bus->UnsubscribeTargeted(event_type_, target_.value(), handler_id_);
    } else {
      bus->Unsubscribe(event_type_, handler_id_);
    }
  }

  unsubscribed_ = true;
}

void EventBus::Unsubscribe(std::type_index event_type, uint64_t handler_id) {
  std::unique_lock<std::mutex> lock(handlers_mutex_);
  auto event_it = event_handlers_.find(event_type);
  if (event_it != event_handlers_.end()) {
    event_it->second.erase(handler_id);
    if (event_it->second.empty()) {
      event_handlers_.erase(event_it);
    }
  }
}

void EventBus::UnsubscribeTargeted(std::type_index event_type, SubjectID target, uint64_t handler_id) {
  std::unique_lock<std::mutex> lock(handlers_mutex_);
  auto event_it = targeted_handlers_.find(event_type);
  if (event_it != targeted_handlers_.end()) {
    auto target_it = event_it->second.find(target);
    if (target_it != event_it->second.end()) {
      target_it->second.erase(handler_id);

      if (target_it->second.empty()) {
        event_it->second.erase(target_it);
      }
      if (event_it->second.empty()) {
        targeted_handlers_.erase(event_it);
      }
    }
  }
}
