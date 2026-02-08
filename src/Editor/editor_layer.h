#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <d3d12.h>

#include "Graphic/Descriptor/descriptor_heap_allocator.h"

class Graphic;
class IScene;
class GameObject;
class TransformComponent;
class SpriteRenderer;
class UISpriteRenderer;
class TextRenderer;
class UITextRenderer;
class MeshRenderer;

class EditorLayer {
 public:
  void Initialize(HWND hwnd, Graphic& graphic);
  void Shutdown(Graphic& graphic);
  void BeginFrame();
  void Render(ID3D12GraphicsCommandList* cmd);
  void SetScene(IScene* scene);
  bool WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

 private:
  void DrawDockSpace();
  void DrawFpsCounter();
  void DrawHierarchy();
  void DrawInspector();
  void DrawTransformInspector(TransformComponent* transform);
  void DrawSpriteRendererInspector(SpriteRenderer* renderer);
  void DrawUISpriteRendererInspector(UISpriteRenderer* renderer);
  void DrawTextRendererInspector(TextRenderer* renderer);
  void DrawUITextRendererInspector(UITextRenderer* renderer);
  void DrawMeshRendererInspector(MeshRenderer* renderer);
  void DrawGameObjectNode(GameObject* go);

  IScene* scene_ = nullptr;
  GameObject* selected_object_ = nullptr;
  DescriptorHeapAllocator::Allocation imgui_font_srv_;
};
