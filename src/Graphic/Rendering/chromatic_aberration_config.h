#pragma once

// Transient runtime effect — do NOT add to GraphicInitProps or config files.
// Mutate only via Graphic::GetChromaticAberrationConfig() from game layer triggers.
struct ChromaticAberrationConfig {
  float intensity = 0.0f;
};
