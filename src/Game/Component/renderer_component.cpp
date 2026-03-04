#include "renderer_component.h"

#include "Game/game_object.h"
#include "Game/scene.h"
#include "Game/System/render_system.h"

namespace detail {

void RegisterRenderable(GameObject* owner, IRenderable* renderable) {
  owner->GetScene()->GetRenderSystem().Register(renderable);
}

void UnregisterRenderable(GameObject* owner, IRenderable* renderable) {
  owner->GetScene()->GetRenderSystem().Unregister(renderable);
}

}  // namespace detail
