#include "graphic.h"

#include <DirectXMath.h>
#include <d3d12.h>
#include <d3dcommon.h>
#include <d3dcompiler.h>
#include <dxgiformat.h>

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>

#include "WICTextureLoader12.h"
#include "d3dx12.h"
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

  HRESULT hr;

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
    root_signature_ = RootSignatureBuilder()
                        .AllowInputLayout()
                        .AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
                          1,
                          0,  // register(t0)
                          D3D12_SHADER_VISIBILITY_PIXEL)
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

#pragma region debug_load_texture
  // Load Texture
  std::unique_ptr<uint8_t[]> decodedData;
  D3D12_SUBRESOURCE_DATA subresource;
  hr = LoadWICTextureFromFile(
    device_.Get(), L"Content/textures/metal_plate_diff_1k.png", texture_buffer_.GetAddressOf(), decodedData, subresource);

  if (FAILED(hr) || texture_buffer_ == nullptr) {
    std::cerr << "Failed to load texture." << std::endl;
    return false;
  }

  D3D12_RESOURCE_DESC textureDesc = texture_buffer_->GetDesc();

  // Configure footprint
  D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint = {};
  UINT numRows = 0;
  UINT64 rowSizeInBytes = 0;
  UINT64 totalBytes = 0;

  device_->GetCopyableFootprints(&textureDesc,
    0,                // FirstSubresource
    1,                // NumSubresources
    0,                // BaseOffset
    &footprint,       // output layout structure
    &numRows,         // output row count (Height)
    &rowSizeInBytes,  // output actual data size per row (excluding alignment Padding)
    &totalBytes       // output total Buffer size
  );

  // Create upload buffer
  CD3DX12_HEAP_PROPERTIES uploadHeapProps(D3D12_HEAP_TYPE_UPLOAD);
  auto uploadBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(totalBytes);

  ComPtr<ID3D12Resource> textureUpload;
  hr = device_->CreateCommittedResource(
    &uploadHeapProps, D3D12_HEAP_FLAG_NONE, &uploadBufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&textureUpload));

  if (FAILED(hr)) {
    std::cerr << "Failed to create texture upload buffer." << std::endl;
    return false;
  }

  // Copy CPU data to upload buffer
  UINT8* pMappedData = nullptr;
  hr = textureUpload->Map(0, nullptr, reinterpret_cast<void**>(&pMappedData));
  if (FAILED(hr)) {
    std::cerr << "Failed to map texture upload buffer." << std::endl;
    return false;
  }

  const UINT8* pSrcData = reinterpret_cast<const UINT8*>(subresource.pData);
  for (UINT y = 0; y < numRows; ++y) {
    memcpy(pMappedData + footprint.Offset + y * footprint.Footprint.RowPitch,
      pSrcData + y * subresource.RowPitch,
      static_cast<size_t>(rowSizeInBytes));
  }
  textureUpload->Unmap(0, nullptr);

  command_allocator_->Reset();
  command_list_->Reset(command_allocator_.Get(), nullptr);

  D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
  srcLocation.pResource = textureUpload.Get();
  srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
  srcLocation.PlacedFootprint = footprint;

  D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
  dstLocation.pResource = texture_buffer_.Get();
  dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
  dstLocation.SubresourceIndex = 0;

  command_list_->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr);

  auto texBarrier =
    CD3DX12_RESOURCE_BARRIER::Transition(texture_buffer_.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
  command_list_->ResourceBarrier(1, &texBarrier);
  command_list_->Close();

  ID3D12CommandList* uploadCmds[] = {command_list_.Get()};
  command_queue_->ExecuteCommandLists(1, uploadCmds);

  fence_manager_.WaitForGpu(command_queue_.Get());

  // texture shader resource heap
  D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
  descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;                              // シェーダから見えるように
  descHeapDesc.NodeMask = 0;                                                                   // マスクは0
  descHeapDesc.NumDescriptors = 1;                                                             // ビューは今のところ１つだけ
  descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;                                  // シェーダリソースビュー(および定数、UAVも)
  hr = device_->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&texture_descriptor_heap_));  // 生成

  if (FAILED(hr) || texture_descriptor_heap_ == nullptr) {
    std::cerr << "Failed to create texture descriptor heap." << std::endl;
    return false;
  }

  // 通常テクスチャビュー作成
  D3D12_RESOURCE_DESC texDesc = texture_buffer_->GetDesc();
  D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
  srvDesc.Format = texDesc.Format;                                             // DXGI_FORMAT_R8G8B8A8_UNORM;//RGBA(0.0f～1.0fに正規化)
  srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;  // 後述
  srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;                       // 2Dテクスチャ
  srvDesc.Texture2D.MipLevels = texDesc.MipLevels;                             // ミップマップは使用しないので1

  device_->CreateShaderResourceView(texture_buffer_.Get(),          // ビューと関連付けるバッファ
    &srvDesc,                                                       // 先ほど設定したテクスチャ設定情報
    texture_descriptor_heap_->GetCPUDescriptorHandleForHeapStart()  // ヒープのどこに割り当てるか
  );
#pragma endregion debug_load_texture

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

  command_list_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  quadMesh_.Draw(command_list_.Get());

  command_list_->SetGraphicsRootSignature(root_signature_.Get());

  ID3D12DescriptorHeap* heaps[] = {texture_descriptor_heap_.Get()};
  command_list_->SetDescriptorHeaps(_countof(heaps), heaps);
  command_list_->SetGraphicsRootDescriptorTable(0, texture_descriptor_heap_->GetGPUDescriptorHandleForHeapStart());

  command_list_->DrawIndexedInstanced(6, 1, 0, 0, 0);
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
