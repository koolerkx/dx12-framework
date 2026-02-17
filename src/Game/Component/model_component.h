#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Asset/model_data.h"
#include "Component/Renderer/mesh_renderer.h"
#include "Component/component.h"
#include "Component/transform_component.h"
#include "Framework/Model/node_hierarchy.h"
#include "Graphic/Frame/render_layer.h"
#include "Graphic/Pipeline/shader_descriptors.h"
#include "Graphic/Pipeline/shader_registry.h"
#include "game_context.h"
#include "game_object.h"
#include "scene.h"

class ModelComponent : public Component<ModelComponent> {
 public:
  struct Props {
    std::shared_ptr<ModelData> model;
    Graphics::ShaderId shader_id = Graphics::PBRShader::ID;
    RenderLayer render_layer = RenderLayer::Opaque;
    bool split_mesh_to_children = false;
    float model_scale = 1.0f;
    bool anchor_to_ground = false;
  };

  using Component::Component;

  ModelComponent(GameObject* owner, const Props& props)
      : Component(owner),
        model_(props.model),
        shader_id_(props.shader_id),
        render_layer_(props.render_layer),
        split_mesh_to_children_(props.split_mesh_to_children),
        model_scale_(props.model_scale),
        anchor_to_ground_(props.anchor_to_ground) {
    if (model_) model_path_ = model_->path;
  }

  void OnStart() override {
    if (model_) {
      BuildNodeTree(model_->root_node, GetOwner());
      ApplyGroundAnchor();
    }
  }

  void OnDestroy() override {
    ClearChildren();
  }

  void LoadModel(std::shared_ptr<ModelData> model) {
    model_ = std::move(model);
    if (model_) model_path_ = model_->path;
    if (GetOwner()->IsStarted()) {
      ClearChildren();
      if (model_) {
        BuildNodeTree(model_->root_node, GetOwner());
        ApplyGroundAnchor();
      }
    }
  }

  const std::shared_ptr<ModelData>& GetModelData() const {
    return model_;
  }

  Graphics::ShaderId GetShaderId() const {
    return shader_id_;
  }
  float GetModelScale() const {
    return model_scale_;
  }

  void OnSerialize(framework::SerializeNode& node) const override {
    node.Write("ModelPath", model_path_);

    auto shader_name = ShaderRegistry::GetName(shader_id_);
    node.Write("Shader", std::string(shader_name));
    node.Write("RenderLayer", render_layer_ == RenderLayer::Opaque ? "Opaque" : "Transparent");

    node.Write("SplitMesh", split_mesh_to_children_);
    node.Write("ModelScale", model_scale_);
    node.Write("AnchorToGround", anchor_to_ground_);
  }

  void OnDeserialize(const framework::SerializeNode& node) override {
    model_path_ = node.ReadString("ModelPath");
    if (!model_path_.empty()) {
      auto* context = GetOwner()->GetContext();
      if (context) {
        auto model = context->GetAssetManager().LoadModel(model_path_);
        if (model) model_ = std::move(model);
      }
    }

    auto shader_name = node.ReadString("Shader", "PBR");
    if (!shader_name.empty()) {
      auto id = ShaderRegistry::FindIdByName(shader_name);
      if (id) shader_id_ = *id;
    }

    auto layer_str = node.ReadString("RenderLayer", "Opaque");
    render_layer_ = (layer_str == "Opaque") ? RenderLayer::Opaque : RenderLayer::Transparent;
    split_mesh_to_children_ = node.ReadBool("SplitMesh", false);
    model_scale_ = node.ReadFloat("ModelScale", 1.0f);
    anchor_to_ground_ = node.ReadBool("AnchorToGround", false);
  }

 private:
  void ApplyGroundAnchor() {
    if (!anchor_to_ground_ || !model_) return;
    auto* transform = GetOwner()->GetTransform();
    if (!transform) return;
    auto anchor = transform->GetAnchor();
    anchor.y = model_->min_y * model_scale_;
    transform->SetAnchor(anchor);
  }

  void BuildNodeTree(const Model::Node& node, GameObject* parent, bool is_root = true) {
    for (const auto& child_node : node.children) {
      auto* scene = GetOwner()->GetScene();

      float scale_factor = (is_root && model_scale_ != 1.0f) ? model_scale_ : 1.0f;

      TransformComponent::Props transform_props;
      transform_props.position = {
        child_node.transform.translation.x * scale_factor,
        child_node.transform.translation.y * scale_factor,
        child_node.transform.translation.z * scale_factor,
      };
      transform_props.scale = {
        child_node.transform.scale.x * scale_factor,
        child_node.transform.scale.y * scale_factor,
        child_node.transform.scale.z * scale_factor,
      };

      std::string node_name = parent->GetName() + "_" + child_node.name;
      auto* child_go = scene->CreateGameObject(node_name, transform_props);
      child_go->SetTransient(true);
      child_go->SetParent(parent);

      auto& rot = child_node.transform.rotation;
      child_go->GetTransform()->SetRotation(Quaternion(rot.x, rot.y, rot.z, rot.w));

      created_children_.push_back(child_go);

      for (size_t i = 0; i < child_node.mesh_indices.size(); ++i) {
        uint32_t mesh_index = child_node.mesh_indices[i];
        if (mesh_index < model_->sub_meshes.size()) {
          const auto& entry = model_->sub_meshes[mesh_index];

          MeshRenderer::Props renderer_props;
          renderer_props.mesh = entry.mesh;
          renderer_props.texture = entry.albedo_texture.get();
          renderer_props.shader_id = shader_id_;
          renderer_props.render_layer = render_layer_;
          renderer_props.normal_texture = entry.normal_texture.get();
          renderer_props.metallic_roughness_texture = entry.metallic_roughness_texture.get();
          renderer_props.emissive_texture = entry.emissive_texture.get();

          if (entry.surface_material_index < model_->surface_materials.size()) {
            const auto& mat = model_->surface_materials[entry.surface_material_index];
            renderer_props.color = {
              mat.base_color_factor.x,
              mat.base_color_factor.y,
              mat.base_color_factor.z,
              mat.base_color_factor.w,
            };
            renderer_props.metallic = mat.metallic_factor;
            renderer_props.roughness = mat.roughness_factor;
            renderer_props.emissive_color = {mat.emissive_factor.x, mat.emissive_factor.y, mat.emissive_factor.z};
          }

          if (split_mesh_to_children_) {
            std::string mesh_name = node_name + "_mesh" + std::to_string(i);
            auto* mesh_go = scene->CreateGameObject(mesh_name);
            mesh_go->SetTransient(true);
            mesh_go->SetParent(child_go);
            mesh_go->AddComponent<MeshRenderer>(renderer_props);
            created_children_.push_back(mesh_go);
          } else {
            child_go->AddComponent<MeshRenderer>(renderer_props);
          }
        }
      }

      BuildNodeTree(child_node, child_go, false);
    }
  }

  void ClearChildren() {
    for (auto* child : created_children_) {
      if (child && !child->IsPendingDestroy()) {
        child->Destroy();
      }
    }
    created_children_.clear();
  }

  std::shared_ptr<ModelData> model_;
  std::string model_path_;
  Graphics::ShaderId shader_id_ = Graphics::PBRShader::ID;
  RenderLayer render_layer_ = RenderLayer::Opaque;
  bool split_mesh_to_children_ = false;
  float model_scale_ = 1.0f;
  bool anchor_to_ground_ = false;
  std::vector<GameObject*> created_children_;
};
