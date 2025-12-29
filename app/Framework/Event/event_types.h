/**
 * @file event_types.h
 * @brief Pre-defined event types for common game systems
 * @author Kooler Fan
 */

#pragma once

#include <cstdint>
#include <string>

namespace Event {

// ============================================================================
// Base Event Structures (Optional - for common fields)
// ============================================================================

struct EventBase {
  uint64_t timestamp_us = 0;
  uint64_t frame_index = 0;
};

// ============================================================================
// Application Events
// ============================================================================

struct ApplicationStartEvent {
  int argc;
  char** argv;
};

struct ApplicationQuitEvent {
  int exit_code = 0;
};

struct WindowResizeEvent {
  int width;
  int height;
  int previous_width;
  int previous_height;
};

struct WindowFocusEvent {
  bool gained_focus;
};

struct WindowCloseEvent {
  bool can_cancel;
  bool* cancelled = nullptr;  // Set to true to prevent close
};

// ============================================================================
// Scene/Gameplay Events
// ============================================================================

struct SceneLoadEvent {
  std::string scene_name;
  int scene_id;
};

struct SceneUnloadEvent {
  std::string scene_name;
  int scene_id;
};

struct GamePauseEvent {
  bool paused;
};

struct GameOverEvent {
  int final_score;
  bool victory;
};

// ============================================================================
// Entity/Actor Events
// ============================================================================

struct EntitySpawnEvent {
  uint64_t entity_id;
  std::string entity_type;
  float x, y, z;
};

struct EntityDestroyEvent {
  uint64_t entity_id;
};

struct EntityCollisionEvent {
  uint64_t entity_a;
  uint64_t entity_b;
  float impact_force;
  float normal_x, normal_y, normal_z;
};

struct HealthChangeEvent {
  uint64_t entity_id;
  float old_health;
  float new_health;
  float damage_amount;
  uint64_t damage_source;
};

struct EntityDeathEvent {
  uint64_t entity_id;
  uint64_t killer_id;
};

// ============================================================================
// Audio Events
// ============================================================================

struct PlaySoundEvent {
  std::string sound_name;
  float volume = 1.0f;
  bool loop = false;
  float x = 0.0f, y = 0.0f, z = 0.0f;  // 3D position
};

struct StopSoundEvent {
  std::string sound_name;
};

struct MusicChangeEvent {
  std::string music_name;
  float fade_duration = 0.0f;
};

// ============================================================================
// UI Events
// ============================================================================

struct ButtonClickEvent {
  std::string button_id;
  int mouse_button;
};

struct MenuOpenEvent {
  std::string menu_id;
};

struct MenuCloseEvent {
  std::string menu_id;
};

struct DialogueStartEvent {
  std::string dialogue_id;
  std::string speaker_name;
};

struct DialogueEndEvent {
  std::string dialogue_id;
};

// ============================================================================
// Network Events
// ============================================================================

struct NetworkConnectEvent {
  std::string server_address;
  uint16_t port;
  bool success;
};

struct NetworkDisconnectEvent {
  std::string reason;
};

struct NetworkMessageEvent {
  uint32_t message_type;
  const void* data;
  size_t size;
};

// ============================================================================
// Save/Load Events
// ============================================================================

struct SaveGameEvent {
  std::string save_slot;
  bool auto_save;
};

struct LoadGameEvent {
  std::string save_slot;
};

struct SaveCompleteEvent {
  std::string save_slot;
  bool success;
};

struct LoadCompleteEvent {
  std::string save_slot;
  bool success;
};

// ============================================================================
// Debug/Editor Events
// ============================================================================

struct DebugCommandEvent {
  std::string command;
  std::string args;
};

struct ConsoleOpenEvent {
  bool open;
};

struct DebugDrawEvent {
  enum class Type { Line, Box, Sphere, Text };
  Type type;
  float duration = 0.0f;
};

// ============================================================================
// Custom Event Example (User-defined)
// ============================================================================

struct PlayerLevelUpEvent {
  int player_id;
  int old_level;
  int new_level;
  int skill_points_gained;
};

struct AchievementUnlockedEvent {
  std::string achievement_id;
  std::string achievement_name;
};

struct QuestCompleteEvent {
  int quest_id;
  std::string quest_name;
  int experience_reward;
  int gold_reward;
};

}  // namespace evt
