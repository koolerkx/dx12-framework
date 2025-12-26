#include "scene.h"

void Scene::AddSprite(const SpriteObject& sprite) {
  sprites_.push_back(sprite);
  sprites_.back().entity_id = next_entity_id_++;
}

RenderWorld Scene::BuildRenderWorld() const {
  RenderWorld world;
  world.ui_sprites.reserve(sprites_.size());

  for (const auto& sprite : sprites_) {
    UiSpriteItem item;
    item.pos = sprite.pos;
    item.size = sprite.size;
    item.tex = sprite.tex;
    item.color = sprite.color;
    world.ui_sprites.push_back(item);
  }

  return world;
}
