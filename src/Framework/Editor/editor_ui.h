/**
 * @file editor_ui.h
 * @brief Abstract editor UI primitives that wrap ImGui without depending on it.
 */
#pragma once

namespace editor_ui {

bool DragFloat(const char* label, float* v, float speed = 1.0f, float min = 0.0f, float max = 0.0f);
bool DragFloat2(const char* label, float* v, float speed = 1.0f, float min = 0.0f, float max = 0.0f);
bool DragFloat3(const char* label, float* v, float speed = 1.0f, float min = 0.0f, float max = 0.0f);
bool DragInt(const char* label, int* v, float speed = 1.0f, int min = 0, int max = 0);
bool SliderFloat(const char* label, float* v, float min, float max);
bool ColorEdit3(const char* label, float* col);
bool ColorEdit4(const char* label, float* col);
bool Checkbox(const char* label, bool* v);
bool InputText(const char* label, char* buf, size_t buf_size);
bool Combo(const char* label, int* current, const char* const items[], int count);
bool Button(const char* label);
bool SmallButton(const char* label);

bool CollapsingHeader(const char* label, bool default_open = false);
bool BeginCombo(const char* label, const char* preview);
void EndCombo();
bool Selectable(const char* label, bool selected = false);

void Text(const char* fmt, ...);
void TextDisabled(const char* fmt, ...);
void SeparatorText(const char* label);
void Separator();
void SameLine();

void PushID(const void* ptr);
void PushID(int id);
void PopID();

void BeginDisabled(bool disabled = true);
void EndDisabled();

void SetNextItemWidth(float width);

bool IsItemActive();
bool IsItemDeactivatedAfterEdit();
bool IsItemHovered();
void SetTooltip(const char* fmt, ...);

}  // namespace editor_ui
