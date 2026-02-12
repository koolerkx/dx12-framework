#pragma once

#include "scene.h"

class BlankScene : public IScene {
 public:
  void OnEnter(AssetManager&) override {
    SetSceneName("Untitled");
  }

  void OnDebugDraw(DebugDrawer& drawer) override {
    DebugDrawer::GridConfig grid_config;
    grid_config.size = 20.0f;
    grid_config.cell_size = 1.0f;
    grid_config.y_level = 0.0f;
    grid_config.color = colors::Gray;
    drawer.DrawGrid(grid_config);

    DebugDrawer::AxisGizmoConfig axis_config;
    axis_config.position = Vector3::Zero;
    axis_config.length = 2.0f;
    drawer.DrawAxisGizmo(axis_config);
  }

  void OnExit() override {
  }
};
