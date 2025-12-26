#include "scene.h"

void Scene::AddSprite(const SpriteObject& sprite) {
  sprites_.push_back(sprite);
  sprites_.back().entity_id = next_entity_id_++;
}

RenderWorld Scene::BuildRenderWorld() const {
  RenderWorld world;
  world.ui_sprites.reserve(sprites_.size());

  for (const auto& sprite : sprites_) {
    world.ui_sprites.emplace_back(UiSpriteItem{.pos = sprite.pos, .size = sprite.size, .tex = sprite.tex, .color = sprite.color});
  }

  return world;
}
