#pragma once

#include "System/system.h"

class IScene;

class WaveSystem : public ISystem {
 public:
  explicit WaveSystem(IScene* scene);
  void Update(float dt) override;

 private:
  void StartWave();
  IScene* scene_ = nullptr;
};
