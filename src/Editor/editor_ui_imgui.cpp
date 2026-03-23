#include <imgui.h>

#include "Framework/Editor/editor_ui.h"

namespace editor_ui {

bool DragFloat(const char* label, float* v, float speed, float min, float max) {
  return ImGui::DragFloat(label, v, speed, min, max);
}

bool DragFloat2(const char* label, float* v, float speed, float min, float max) {
  return ImGui::DragFloat2(label, v, speed, min, max);
}

bool DragFloat3(const char* label, float* v, float speed, float min, float max) {
  return ImGui::DragFloat3(label, v, speed, min, max);
}

bool DragInt(const char* label, int* v, float speed, int min, int max) {
  return ImGui::DragInt(label, v, speed, min, max);
}

bool SliderFloat(const char* label, float* v, float min, float max) {
  return ImGui::SliderFloat(label, v, min, max);
}

bool ColorEdit3(const char* label, float* col) {
  return ImGui::ColorEdit3(label, col);
}

bool ColorEdit4(const char* label, float* col) {
  return ImGui::ColorEdit4(label, col);
}

bool Checkbox(const char* label, bool* v) {
  return ImGui::Checkbox(label, v);
}

bool InputText(const char* label, char* buf, size_t buf_size) {
  return ImGui::InputText(label, buf, buf_size);
}

bool Combo(const char* label, int* current, const char* const items[], int count) {
  return ImGui::Combo(label, current, items, count);
}

bool Button(const char* label) {
  return ImGui::Button(label);
}

bool SmallButton(const char* label) {
  return ImGui::SmallButton(label);
}

bool CollapsingHeader(const char* label, bool default_open) {
  ImGuiTreeNodeFlags flags = default_open ? ImGuiTreeNodeFlags_DefaultOpen : 0;
  return ImGui::CollapsingHeader(label, flags);
}

bool BeginCombo(const char* label, const char* preview) {
  return ImGui::BeginCombo(label, preview);
}

void EndCombo() {
  ImGui::EndCombo();
}

bool Selectable(const char* label, bool selected) {
  return ImGui::Selectable(label, selected);
}

void Text(const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  ImGui::TextV(fmt, args);
  va_end(args);
}

void TextDisabled(const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  ImGui::TextDisabledV(fmt, args);
  va_end(args);
}

void SeparatorText(const char* label) {
  ImGui::SeparatorText(label);
}

void Separator() {
  ImGui::Separator();
}

void SameLine() {
  ImGui::SameLine();
}

void PushID(const void* ptr) {
  ImGui::PushID(ptr);
}

void PushID(int id) {
  ImGui::PushID(id);
}

void PopID() {
  ImGui::PopID();
}

void BeginDisabled(bool disabled) {
  ImGui::BeginDisabled(disabled);
}

void EndDisabled() {
  ImGui::EndDisabled();
}

void SetNextItemWidth(float width) {
  ImGui::SetNextItemWidth(width);
}

bool IsItemActive() {
  return ImGui::IsItemActive();
}

bool IsItemDeactivatedAfterEdit() {
  return ImGui::IsItemDeactivatedAfterEdit();
}

bool IsItemHovered() {
  return ImGui::IsItemHovered();
}

void SetTooltip(const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  ImGui::SetTooltipV(fmt, args);
  va_end(args);
}

}  // namespace editor_ui
