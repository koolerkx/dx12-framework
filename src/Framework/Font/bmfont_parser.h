#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace BMFont {

// BMFont file format structures
struct BmFontInfo {
  std::string face;
  int32_t size = 0;
  bool bold = false;
  bool italic = false;
  std::string charset;
  bool unicode = false;
  int32_t stretchH = 100;
  bool smooth = false;
  int32_t aa = 1;
  int32_t padding[4] = {0, 0, 0, 0};
  int32_t spacing[2] = {0, 0};
  int32_t outline = 0;
};

struct BmFontCommon {
  uint16_t lineHeight = 0;
  uint16_t base = 0;
  uint16_t scaleW = 0;
  uint16_t scaleH = 0;
  uint16_t pages = 0;
  bool packed = false;
  uint8_t alphaChnl = 0;
  uint8_t redChnl = 0;
  uint8_t greenChnl = 0;
  uint8_t blueChnl = 0;
};

struct BmFontPage {
  uint16_t id = 0;
  std::string file;
};

struct BmFontChar {
  uint32_t id = 0;
  uint16_t x = 0;
  uint16_t y = 0;
  uint16_t width = 0;
  uint16_t height = 0;
  int16_t xoffset = 0;
  int16_t yoffset = 0;
  int16_t xadvance = 0;
  uint8_t page = 0;
  uint8_t chnl = 0;
};

struct BmFontKerning {
  uint32_t first = 0;
  uint32_t second = 0;
  int16_t amount = 0;
};

struct BmFontData {
  BmFontInfo info;
  BmFontCommon common;
  std::vector<BmFontPage> pages;
  std::unordered_map<uint32_t, BmFontChar> chars;
  std::unordered_map<uint64_t, int16_t> kernings;  // Key: (first << 32 | second)

  // Helper to get kerning amount
  int16_t GetKerning(uint32_t first, uint32_t second) const {
    uint64_t key = (static_cast<uint64_t>(first) << 32) | second;
    auto it = kernings.find(key);
    return it != kernings.end() ? it->second : 0;
  }
};

// Parser function
bool ParseBmFontText(const std::string& filepath, BmFontData& out_data);

}  // namespace bmfont
