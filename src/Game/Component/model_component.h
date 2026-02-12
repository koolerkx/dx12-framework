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
#include "game_object.h"
#include "scene.h"

class ModelComponent : public Component<ModelComponent> {
 public:
  struct Props {
    std::shared_ptr<ModelData> model;
    Graphics::ShaderId shader_id = Graphics::PBRShader::ID;
    RenderLayer render_layer = RenderLayer::Opaque;
    bool split_mesh_to_children = false;
  };

  ModelComponent(GameObject* owner, const Props& props)
      : Component(owner),
        model_(props.model),
        shader_id_(props.shader_id),
        render_layer_(props.render_layer),
        split_mesh_to_children_(props.split_mesh_to_children) {
  }

  void OnStart() override {
    if (model_) {
      BuildNodeTree(model_->root_node, GetOwner());
    }
  }

  void OnDestroy() override {
    ClearChildren();
  }

  void LoadModel(std::shared_ptr<ModelData> model) {
    model_ = std::move(model);
    if (GetOwner()->IsStarted()) {
      ClearChildren();
      if (model_) {
        BuildNodeTree(model_->root_node, GetOwner());
      }
    }
  }

  const std::shared_ptr<ModelData>& GetModelData() const {
    return model_;
  }

 private:
  void BuildNodeTree(const Model::Node& node, GameObject* parent) {
    for (const auto& child_node : node.children) {
      auto* scene = GetOwner()->GetScene();

      TransformComponent::Props transform_props;
      transform_props.position = {
        child_node.transform.translation.x,
        child_node.transform.translation.y,
        child_node.transform.translation.z,
      };
      transform_props.scale = {
        child_node.transform.scale.x,
        child_node.transform.scale.y,
        child_node.transform.scale.z,
      };

      auto* child_go = scene->CreateGameObject(child_node.name, transform_props);
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
            std::string mesh_name = child_node.name + "_mesh" + std::to_string(i);
            auto* mesh_go = scene->CreateGameObject(mesh_name);
            mesh_go->SetParent(child_go);
            mesh_go->AddComponent<MeshRenderer>(renderer_props);
            created_children_.push_back(mesh_go);
          } else {
            child_go->AddComponent<MeshRenderer>(renderer_props);
          }
        }
      }

      BuildNodeTree(child_node, child_go);
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
  Graphics::ShaderId shader_id_ = Graphics::PBRShader::ID;
  RenderLayer render_layer_ = RenderLayer::Opaque;
  bool split_mesh_to_children_ = false;
  std::vector<GameObject*> created_children_;
};
