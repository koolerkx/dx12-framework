#include "graphic.h"

#include <DirectXMath.h>
#include <d3d12.h>
#include <d3dcommon.h>
#include <d3dcompiler.h>
#include <dxgiformat.h>

#include <cstdint>
#include <iostream>
#include <memory>
#include <vector>

#include "pipeline_state_builder.h"
#include "root_signature_builder.h"
#include "types.h"

using namespace DirectX;

struct Vertex {
  XMFLOAT3 pos;
  XMFLOAT2 uv;
};

struct TexRGBA {
  unsigned char R, G, B, A;
};

bool Graphic::Initalize(HWND hwnd, UINT frame_buffer_width, UINT frame_buffer_height) {
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
  if (!descriptor_heap_manager_.Initalize(device_.Get())) {
    MessageBoxW(nullptr, L"Graphic: Failed to initialize descriptor heap manager", init_error_caption.c_str(), MB_OK | MB_ICONERROR);
    return false;
  }

  if (!CreateCommandQueue()) {
    MessageBoxW(nullptr, L"Graphic: Failed to create command queue", init_error_caption.c_str(), MB_OK | MB_ICONERROR);
    return false;
  }
  if (!CreateCommandAllocator()) {
    MessageBoxW(nullptr, L"Graphic: Failed to create command allocator", init_error_caption.c_str(), MB_OK | MB_ICONERROR);
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

  // Draw Triangle
  Vertex vertices[] = {
    {{-0.4f, -0.7f, 0.0f}, {0.0f, 1.0f}},  // 左下
    {{-0.4f, 0.7f, 0.0f}, {0.0f, 0.0f}},   // 左上
    {{0.4f, -0.7f, 0.0f}, {1.0f, 1.0f}},   // 右下
    {{0.4f, 0.7f, 0.0f}, {1.0f, 0.0f}},    // 右上
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

  if (FAILED(D3DReadFileToBlob(L"Content/shaders/basic.vs.cso", &vsBlob))) {
    throw std::runtime_error("Failed to read vertex shader");
  }

  if (FAILED(D3DReadFileToBlob(L"Content/shaders/basic.ps.cso", &psBlob))) {
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
                        .AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
                          srv_capacity,
                          0,  // register(t0)
                          1,
                          D3D12_SHADER_VISIBILITY_ALL)
                        .Add32BitConstants(1, 0)
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

  command_allocator_->Reset();
  command_list_->Reset(command_allocator_.Get(), nullptr);

  myTexture = texture_manager_.LoadTextures(
    std::vector<std::wstring>{L"Content/textures/metal_plate_diff_1k.png", L"Content/textures/metal_plate_disp_1k.png"});

  command_list_->Close();
  ID3D12CommandList* cmds[] = {command_list_.Get()};
  command_queue_->ExecuteCommandLists(1, cmds);
  fence_manager_.WaitForGpu(command_queue_.Get());

  return true;
}

void Graphic::BeginRender() {
  command_allocator_->Reset();
  command_list_->Reset(command_allocator_.Get(), nullptr);

  descriptor_heap_manager_.BeginFrame();
  descriptor_heap_manager_.SetDescriptorHeaps(command_list_.Get());

  swap_chain_manager_.TransitionToRenderTarget(command_list_.Get());

  D3D12_CPU_DESCRIPTOR_HANDLE rtv = swap_chain_manager_.GetCurrentRTV();
  D3D12_CPU_DESCRIPTOR_HANDLE dsv = depth_buffer_.GetDSV();

  command_list_->RSSetViewports(1, &viewport_);
  command_list_->RSSetScissorRects(1, &scissor_rect_);

  float clearColor[] = {1.0f, 1.0f, 0.0f, 1.0f};
  command_list_->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
  depth_buffer_.Clear(command_list_.Get(), 1.0, 0);

  command_list_->OMSetRenderTargets(1, &rtv, FALSE, &dsv);

  // Drawing Logic
  command_list_->SetPipelineState(pipeline_state_.Get());
  command_list_->RSSetViewports(1, &viewport_);
  command_list_->RSSetScissorRects(1, &scissor_rect_);
  command_list_->SetGraphicsRootSignature(root_signature_.Get());

  command_list_->SetGraphicsRootDescriptorTable(
    0, descriptor_heap_manager_.GetSrvAllocator().GetHeap()->GetGPUDescriptorHandleForHeapStart());
  command_list_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  uint32_t indexA = myTexture[0]->GetBindlessIndex();
  command_list_->SetGraphicsRoot32BitConstants(1, 1, &indexA, 0);
  quadMesh_.Draw(command_list_.Get());

  uint32_t indexB = myTexture[1]->GetBindlessIndex();
  command_list_->SetGraphicsRoot32BitConstants(1, 1, &indexB, 0);
  quadMesh_.Draw(command_list_.Get());
}

void Graphic::EndRender() {
  swap_chain_manager_.TransitionToPresent(command_list_.Get());

  command_list_->Close();

  ID3D12CommandList* cmdlists[] = {command_list_.Get()};
  command_queue_->ExecuteCommandLists(1, cmdlists);

  fence_manager_.WaitForGpu(command_queue_.Get());

  swap_chain_manager_.Present(1, 0);
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
  auto hr = device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, command_allocator_.Get(), nullptr, IID_PPV_ARGS(&command_list_));

  if (FAILED(hr) || command_list_ == nullptr) {
    std::cerr << "Failed to create command list." << std::endl;
    return false;
  }

  command_list_->Close();
  return true;
}

bool Graphic::CreateCommandAllocator() {
  auto hr = device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&command_allocator_));

  if (FAILED(hr) || command_allocator_ == nullptr) {
    std::cerr << "Failed to create command allocator." << std::endl;
    return false;
  }

  return true;
}

void Graphic::ExecuteSync(std::function<void(ID3D12GraphicsCommandList*)> cb) {
  command_allocator_->Reset();
  command_list_->Reset(command_allocator_.Get(), nullptr);

  cb(command_list_.Get());

  command_list_->Close();
  ID3D12CommandList* lists[] = {command_list_.Get()};
  command_queue_->ExecuteCommandLists(1, lists);

  fence_manager_.WaitForGpu(command_queue_.Get());
}

uint64_t Graphic::ExecuteAsync(std::function<void(ID3D12GraphicsCommandList*)> cb) {
  std::lock_guard<std::mutex> lock(command_list_mutex_);  // TODO: remove this mutex and assign command allocator for each thread

  command_allocator_->Reset();
  command_list_->Reset(command_allocator_.Get(), nullptr);

  cb(command_list_.Get());

  command_list_->Close();
  ID3D12CommandList* lists[] = {command_list_.Get()};
  command_queue_->ExecuteCommandLists(1, lists);

  return static_cast<uint64_t>(fence_manager_.SignalFence(command_queue_.Get()));
}
