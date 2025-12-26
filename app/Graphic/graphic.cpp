#include "graphic.h"

#include <DirectXMath.h>
#include <d3d12.h>
#include <d3dcommon.h>
#include <d3dcompiler.h>
#include <dxgiformat.h>

#include <cstdint>
#include <iostream>
#include <vector>

#include "Core/types.h"
#include "pipeline_state_builder.h"
#include "root_signature_builder.h"

using namespace DirectX;

constexpr std::array<float, 4> CLEAR_COLOR = {1.0f, 1.0f, 0.0f, 1.0f};

struct Vertex {
  XMFLOAT3 pos;
  XMFLOAT2 uv;
};

struct TexRGBA {
  unsigned char R, G, B, A;
};

bool Graphic::Initialize(HWND hwnd, UINT frame_buffer_width, UINT frame_buffer_height) {
  frame_buffer_width_ = frame_buffer_width;
  frame_buffer_height_ = frame_buffer_height;

  std::wstring init_error_caption = L"Graphic Initialization Error";

  if (!CreateFactory()) {
    MessageBoxW(nullptr, L"Graphic: Failed to create factory", init_error_caption.c_str(), MB_OK | MB_ICONERROR);
    return false;
  }
  if (!CreateDevice()) {
    MessageBoxW(nullptr, L"Graphic: Failed to create device", init_error_caption.c_str(), MB_OK | MB_ICONERROR);
    return false;
  }

  DescriptorHeapConfig heapConfig;
  if (!descriptor_heap_manager_.Initialize(device_.Get(), FRAME_BUFFER_COUNT, heapConfig)) {
    MessageBoxW(nullptr, L"Graphic: Failed to initialize descriptor heap manager", init_error_caption.c_str(), MB_OK | MB_ICONERROR);
    return false;
  }

  if (!CreateCommandQueue()) {
    MessageBoxW(nullptr, L"Graphic: Failed to create command queue", init_error_caption.c_str(), MB_OK | MB_ICONERROR);
    return false;
  }
  if (!CreateCommandAllocators()) {
    MessageBoxW(nullptr, L"Graphic: Failed to create command allocators", init_error_caption.c_str(), MB_OK | MB_ICONERROR);
    return false;
  }
  if (!CreateCommandList()) {
    MessageBoxW(nullptr, L"Graphic: Failed to create command list", init_error_caption.c_str(), MB_OK | MB_ICONERROR);
    return false;
  }

  if (!swap_chain_manager_.Initialize(device_.Get(),
        dxgi_factory_.Get(),
        command_queue_.Get(),
        hwnd,
        frame_buffer_width,
        frame_buffer_height,
        descriptor_heap_manager_)) {
    return false;
  }
  if (!depth_buffer_.Initialize(device_.Get(), frame_buffer_width, frame_buffer_height, descriptor_heap_manager_)) {
    return false;
  }
  if (!fence_manager_.Initialize(device_.Get())) {
    return false;
  }

  if (!texture_manager_.Initialize(this, device_.Get(), &descriptor_heap_manager_)) {
    return false;
  }

  if (!frame_cb_storage_.Initialize(device_.Get(), FRAME_BUFFER_COUNT)) return false;

  size_t object_buffer_page_size = 1024 * 1024;  // 1mb per page

  object_cb_allocators_.resize(FRAME_BUFFER_COUNT);
  for (int i = 0; i < FRAME_BUFFER_COUNT; ++i) {
    object_cb_allocators_[i] = std::make_unique<DynamicUploadBuffer>();
    std::wstring bufferName = L"ObjectCB_Frame" + std::to_wstring(i);
    if (!object_cb_allocators_[i]->Initialize(device_.Get(), object_buffer_page_size, bufferName)) {
      return false;
    }
  }

  // Draw Triangle
  Vertex vertices[] = {
    {{-0.5f, -0.5f, 0.0f}, {0.0f, 1.0f}},  // LB
    {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f}},   // LT
    {{0.5f, -0.5f, 0.0f}, {1.0f, 1.0f}},   // RB
    {{0.5f, 0.5f, 0.0f}, {1.0f, 0.0f}},    // RT
  };

  uint16_t indices[] = {0, 1, 2, 2, 1, 3};

  // Create vertex buffer
  if (!quadMesh_.Create(device_.Get(), vertices, 4, indices, 6)) {
    std::cerr << "Failed to create mesh." << std::endl;
    return false;
  }

  // Read Shader
  ComPtr<ID3DBlob> vsBlob;
  ComPtr<ID3DBlob> psBlob;

  if (FAILED(D3DReadFileToBlob(L"Content/shaders/sprite.vs.cso", &vsBlob))) {
    throw std::runtime_error("Failed to read vertex shader");
  }

  if (FAILED(D3DReadFileToBlob(L"Content/shaders/sprite.ps.cso", &psBlob))) {
    throw std::runtime_error("Failed to read pixel shader");
  }

  // Input layout
  std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout = {
    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}};

  // Graphics pipeline
  try {
    uint32_t srv_capacity = 4096;
    root_signature_ = RootSignatureBuilder()
                        .AllowInputLayout()
                        .AddRootCBV(0, 0)         // FrameCB
                        .AddRootCBV(1, 0)         // ObjectCB
                        .Add32BitConstants(1, 2)  // texture bindless id
                        .AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
                          srv_capacity,
                          0,  // register(t0)
                          1,
                          D3D12_SHADER_VISIBILITY_ALL)
                        .AddStaticSampler(SamplerPresets::CreatePointSampler(0))
                        .Build(device_.Get());

    pipeline_state_ = PipelineStateBuilder()
                        .SetRootSignature(root_signature_.Get())
                        .SetVertexShader(vsBlob.Get())
                        .SetPixelShader(psBlob.Get())
                        .SetInputLayout(inputLayout)
                        .SetRenderTargetFormat(DXGI_FORMAT_R8G8B8A8_UNORM)
                        .SetCullMode(D3D12_CULL_MODE_NONE)
                        .SetBlendMode(BlendMode::AlphaBlend)
                        .Build(device_.Get());

  } catch (const std::exception& e) {
    std::cerr << "Initialization failed: " << e.what() << std::endl;
    return false;
  }

  viewport_.Width = static_cast<FLOAT>(frame_buffer_width_);    // 出力先の幅(ピクセル数)
  viewport_.Height = static_cast<FLOAT>(frame_buffer_height_);  // 出力先の高さ(ピクセル数)
  viewport_.TopLeftX = 0;                                       // 出力先の左上座標X
  viewport_.TopLeftY = 0;                                       // 出力先の左上座標Y
  viewport_.MaxDepth = 1.0f;                                    // 深度最大値
  viewport_.MinDepth = 0.0f;                                    // 深度最小値

  scissor_rect_.top = 0;                                            // 切り抜き上座標
  scissor_rect_.left = 0;                                           // 切り抜き左座標
  scissor_rect_.right = scissor_rect_.left + frame_buffer_width_;   // 切り抜き右座標
  scissor_rect_.bottom = scissor_rect_.top + frame_buffer_height_;  // 切り抜き下座標

  ui_renderer_ = std::make_unique<UiRenderer>(root_signature_.Get(), pipeline_state_.Get(), &quadMesh_);

  ui_pass_ = std::make_unique<UiPass>(ui_renderer_.get());
  render_pass_manager_ = std::make_unique<RenderPassManager>();
  render_pass_manager_->SetUiPass(ui_pass_.get());

  return true;
}

void Graphic::Shutdown() {
  if (command_queue_ && fence_manager_.IsValid()) {
    fence_manager_.WaitForGpu(command_queue_.Get());
  }
}

RenderFrameContext Graphic::BeginFrame() {
  uint32_t frame_index = swap_chain_manager_.GetCurrentBackBufferIndex();

  // Wait for resource
  uint64_t fence_value = frame_fence_values_[frame_index];
  if (fence_value != 0) {
    fence_manager_.WaitForFenceValue(fence_value);
  }

  object_cb_allocators_[frame_index]->Reset();  // manually reset for dynamic allocation

  auto* current_allocator = command_allocators_[frame_index].Get();
  (void)current_allocator->Reset();
  (void)command_list_->Reset(current_allocator, nullptr);

  descriptor_heap_manager_.BeginFrame(frame_index);
  descriptor_heap_manager_.SetDescriptorHeaps(command_list_.Get());  // Bind Global Heap
  swap_chain_manager_.TransitionToRenderTarget(command_list_.Get());

  D3D12_CPU_DESCRIPTOR_HANDLE rtv = swap_chain_manager_.GetCurrentRTV();
  D3D12_CPU_DESCRIPTOR_HANDLE dsv = depth_buffer_.GetDSV();

  command_list_->ClearRenderTargetView(rtv, CLEAR_COLOR.data(), 0, nullptr);
  depth_buffer_.Clear(command_list_.Get(), 1.0, 0);
  command_list_->OMSetRenderTargets(1, &rtv, FALSE, &dsv);

  command_list_->RSSetViewports(1, &viewport_);
  command_list_->RSSetScissorRects(1, &scissor_rect_);

  return RenderFrameContext{.frame_index = frame_index,
    .command_list = command_list_.Get(),

    // Pass the pointer to the specific ConstantBuffer instance
    .frame_cb = &frame_cb_storage_.GetBuffer(frame_index),
    .object_cb_allocator = object_cb_allocators_[frame_index].get(),

    .dynamic_allocator = &descriptor_heap_manager_.GetSrvDynamicAllocator(frame_index),
    .global_heap_manager = &descriptor_heap_manager_,

    .screen_width = frame_buffer_width_,
    .screen_height = frame_buffer_height_};
}

void Graphic::EndFrame(const RenderFrameContext& frame) {
  swap_chain_manager_.TransitionToPresent(frame.command_list);

  (void)frame.command_list->Close();
  ID3D12CommandList* command_lists[] = {frame.command_list};
  command_queue_->ExecuteCommandLists(1, command_lists);

  uint64_t fence_value = fence_manager_.SignalFence(command_queue_.Get());
  frame_fence_values_[frame.frame_index] = fence_value;
  frame_cb_storage_.MarkFrameSubmitted(frame.frame_index, fence_value);

  swap_chain_manager_.Present(1, 0);
}

void Graphic::RenderScene(const RenderFrameContext& frame, const RenderWorld& world) {
  render_pass_manager_->Execute(frame, world);
}

bool Graphic::EnableDebugLayer() {
  ID3D12Debug* debugLayer = nullptr;

  if (FAILED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugLayer)))) {
    return false;
  }

  debugLayer->EnableDebugLayer();
  debugLayer->Release();

  return true;
}

bool Graphic::CreateFactory() {
#if defined(DEBUG) || defined(_DEBUG)
  EnableDebugLayer();
  UINT dxgi_factory_flag = DXGI_CREATE_FACTORY_DEBUG;
#else
  UINT dxgi_factory_flag = 0;
#endif

  auto hr = CreateDXGIFactory2(dxgi_factory_flag, IID_PPV_ARGS(&dxgi_factory_));
  if (FAILED(hr)) {
    return false;
  }
  return true;
}

bool Graphic::CreateDevice() {
  std::vector<ComPtr<IDXGIAdapter>> adapters;
  ComPtr<IDXGIAdapter> tmpAdapter = nullptr;

  for (int i = 0; dxgi_factory_->EnumAdapters(i, &tmpAdapter) != DXGI_ERROR_NOT_FOUND; ++i) {
    adapters.push_back(tmpAdapter);
  }

  for (auto adapter : adapters) {
    DXGI_ADAPTER_DESC adapter_desc = {};
    adapter->GetDesc(&adapter_desc);
    if (std::wstring desc_str = adapter_desc.Description; desc_str.find(L"NVIDIA") != std::string::npos) {
      tmpAdapter = adapter;
      break;
    }
  }

  D3D_FEATURE_LEVEL levels[] = {
    D3D_FEATURE_LEVEL_12_2,
    D3D_FEATURE_LEVEL_12_1,
    D3D_FEATURE_LEVEL_12_0,
    D3D_FEATURE_LEVEL_11_1,
    D3D_FEATURE_LEVEL_11_0,
  };

  for (auto level : levels) {
    auto hr = D3D12CreateDevice(tmpAdapter.Get(), level, IID_PPV_ARGS(&device_));
    if (SUCCEEDED(hr) && device_ != nullptr) {
      return true;
    }
  }

  std::cerr << "Failed to create device." << std::endl;
  return false;
}

bool Graphic::CreateCommandQueue() {
  D3D12_COMMAND_QUEUE_DESC command_queue_desc = {};
  command_queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
  command_queue_desc.NodeMask = 0;
  command_queue_desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
  command_queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

  auto hr = device_->CreateCommandQueue(&command_queue_desc, IID_PPV_ARGS(&command_queue_));
  if (FAILED(hr) || command_queue_ == nullptr) {
    std::cerr << "Failed to create command queue." << std::endl;
    return false;
  }
  return true;
}

bool Graphic::CreateCommandList() {
  auto hr =
    device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, utility_command_allocator_.Get(), nullptr, IID_PPV_ARGS(&command_list_));
  command_list_->SetName(L"Main_Graphics_CommandList");

  if (FAILED(hr) || command_list_ == nullptr) {
    std::cerr << "Failed to create command list." << std::endl;
    return false;
  }

  command_list_->Close();
  return true;
}

bool Graphic::CreateCommandAllocators() {
  auto hr = device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&utility_command_allocator_));

  if (FAILED(hr) || utility_command_allocator_ == nullptr) {
    std::cerr << "Failed to create utility command allocator" << std::endl;
    return false;
  }
  utility_command_allocator_->SetName(L"UtilityCommandAllocator");

  for (int i = 0; i < FRAME_BUFFER_COUNT; ++i) {
    hr = device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&command_allocators_[i]));

    if (FAILED(hr) || command_allocators_[i] == nullptr) {
      std::cerr << "Failed to create command allocator " << i << std::endl;
      return false;
    }

    std::wstring name = L"CommandAllocator_Frame" + std::to_wstring(i);
    command_allocators_[i]->SetName(name.c_str());
  }

  return true;
}

void Graphic::ExecuteSync(std::function<void(ID3D12GraphicsCommandList*)> cb) {
  utility_command_allocator_->Reset();
  command_list_->Reset(utility_command_allocator_.Get(), nullptr);

  cb(command_list_.Get());

  command_list_->Close();
  ID3D12CommandList* lists[] = {command_list_.Get()};
  command_queue_->ExecuteCommandLists(1, lists);

  fence_manager_.WaitForGpu(command_queue_.Get());
}
