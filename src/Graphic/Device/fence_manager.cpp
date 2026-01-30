#include "fence_manager.h"

#include <cassert>

#include "Framework/Logging/logger.h"

bool FenceManager::Initialize(ID3D12Device* device) {
  assert(device != nullptr);

  HRESULT hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_));
  if (FAILED(hr)) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "[FenceManager] Failed to create fence");
    return false;
  }

  fence_value_ = 1;

  fence_event_ = CreateEvent(nullptr, false, false, nullptr);
  if (!fence_event_) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "[FenceManager] Failed to create fence event");
    return false;
  }

  return true;
}

UINT64 FenceManager::SignalFence(ID3D12CommandQueue* command_queue) {
  assert(command_queue != nullptr);
  assert(fence_ != nullptr);

  std::lock_guard<std::mutex> lock(fence_mutex_);
  UINT64 value_to_signal = fence_value_;

  command_queue->Signal(fence_.Get(), value_to_signal);
  fence_value_.fetch_add(1, std::memory_order_release);

  return value_to_signal;
}

void FenceManager::WaitForFenceValue(UINT64 fence_value) {
  assert(fence_ != nullptr);
  assert(fence_event_ != nullptr);

  if (fence_->GetCompletedValue() >= fence_value) {
    return;  // Already completed
  }

  HRESULT hr = fence_->SetEventOnCompletion(fence_value, fence_event_);
  if (FAILED(hr)) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "[FenceManager] SetEventOnCompletion failed: HRESULT = 0x{:08X}",
      static_cast<uint32_t>(hr));

#ifdef _DEBUG
    std::terminate();  // Fail fast in debug
#else
    throw std::runtime_error("Failed to set fence completion event");
#endif
  }

  constexpr DWORD INITIAL_TIMEOUT_MS = 5000;  // 5 seconds
  DWORD wait_result = WaitForSingleObject(fence_event_, INITIAL_TIMEOUT_MS);

  if (wait_result == WAIT_TIMEOUT) {
    Logger::LogFormat(LogLevel::Warn, LogCategory::Graphic, Logger::Here(),
      "[FenceManager] WARNING: Fence wait timeout after {}ms (fence value: {})", INITIAL_TIMEOUT_MS, fence_value);

#ifdef _DEBUG
    // Debug: terminate immediately to catch the issue
    Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "[FenceManager] Debug build: terminating on timeout");
    std::terminate();
#else
    // Release: try extended wait
    constexpr DWORD EXTENDED_TIMEOUT_MS = 30000;  // 30 seconds
    Logger::LogFormat(LogLevel::Warn, LogCategory::Graphic, Logger::Here(), "[FenceManager] Attempting extended wait ({}ms)...",
      EXTENDED_TIMEOUT_MS);

    wait_result = WaitForSingleObject(fence_event_, EXTENDED_TIMEOUT_MS);

    if (wait_result != WAIT_OBJECT_0) {
      Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "[FenceManager] CRITICAL: Extended wait failed! Result: {}",
        wait_result);
      throw std::runtime_error("GPU fence wait timeout - cannot safely proceed");
    }

    Logger::LogFormat(LogLevel::Info, LogCategory::Graphic, Logger::Here(), "[FenceManager] Extended wait succeeded");
#endif

  } else if (wait_result != WAIT_OBJECT_0) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(),
      "[FenceManager] ERROR: WaitForSingleObject failed with code {}, last error: {}", wait_result, GetLastError());

#ifdef _DEBUG
    std::terminate();
#else
    throw std::runtime_error("GPU fence wait failed unexpectedly");
#endif
  }
}

void FenceManager::WaitForGpu(ID3D12CommandQueue* command_queue) {
  assert(command_queue != nullptr);
  assert(fence_ != nullptr);
  assert(fence_event_ != nullptr);

  UINT64 fence_value_to_wait;

  // protect Signal + increment
  {
    std::lock_guard<std::mutex> lock(fence_mutex_);

    fence_value_to_wait = fence_value_.load(std::memory_order_acquire);

    HRESULT hr = command_queue->Signal(fence_.Get(), fence_value_to_wait);
    if (FAILED(hr)) {
      Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(),
        "[FenceManager] CRITICAL: Signal failed in WaitForGpu, HRESULT = 0x{:08X}", static_cast<uint32_t>(hr));

#ifdef _DEBUG
      std::terminate();  // Fail fast in debug
#else
      throw std::runtime_error("GPU signal failed - cannot sync GPU");
#endif
    }

    fence_value_.fetch_add(1, std::memory_order_release);
  }  // Lock released here - safe to wait outside mutex

  // Wait outside of lock to avoid deadlock
  if (fence_->GetCompletedValue() >= fence_value_to_wait) {
    return;  // Already completed
  }

  HRESULT hr = fence_->SetEventOnCompletion(fence_value_to_wait, fence_event_);
  if (FAILED(hr)) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(),
      "[FenceManager] CRITICAL: SetEventOnCompletion failed in WaitForGpu, HRESULT = 0x{:08X}", static_cast<uint32_t>(hr));

#ifdef _DEBUG
    std::terminate();
#else
    throw std::runtime_error("Cannot set GPU completion event");
#endif
  }

  constexpr DWORD INITIAL_TIMEOUT_MS = 5000;
  DWORD wait_result = WaitForSingleObject(fence_event_, INITIAL_TIMEOUT_MS);

  if (wait_result == WAIT_TIMEOUT) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "[FenceManager] CRITICAL: GPU sync timeout after {}ms!",
      INITIAL_TIMEOUT_MS);
    Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "[FenceManager] Fence value: {}, Completed: {}",
      fence_value_to_wait, fence_->GetCompletedValue());

#ifdef _DEBUG
    // Debug: terminate immediately - GPU hang needs investigation
    Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(),
      "[FenceManager] Debug build: terminating to prevent resource corruption");
    std::terminate();
#else
    // Release: try extended wait before giving up
    constexpr DWORD EXTENDED_TIMEOUT_MS = 30000;
    Logger::LogFormat(LogLevel::Warn, LogCategory::Graphic, Logger::Here(), "[FenceManager] Attempting extended wait ({}ms)...",
      EXTENDED_TIMEOUT_MS);

    wait_result = WaitForSingleObject(fence_event_, EXTENDED_TIMEOUT_MS);

    if (wait_result != WAIT_OBJECT_0) {
      Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(),
        "[FenceManager] CRITICAL: Extended GPU sync failed! Result: {}, last error: {}", wait_result, GetLastError());
      Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(),
        "[FenceManager] GPU may still be using resources - UNSAFE TO RELEASE!");

      // Cannot safely release resources
      throw std::runtime_error("GPU sync timeout after extended wait - cannot safely release resources");
    }

    Logger::LogFormat(LogLevel::Warn, LogCategory::Graphic, Logger::Here(),
      "[FenceManager] Extended wait succeeded (slow GPU performance detected)");
#endif

  } else if (wait_result != WAIT_OBJECT_0) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(),
      "[FenceManager] CRITICAL: WaitForSingleObject failed with code {}, last error: {}", wait_result, GetLastError());

#ifdef _DEBUG
    std::terminate();
#else
    throw std::runtime_error("GPU wait failed unexpectedly");
#endif
  }
}

void FenceManager::ShutDown() {
  if (fence_event_) {
    CloseHandle(fence_event_);
    fence_event_ = nullptr;
  }
  fence_.Reset();
}
