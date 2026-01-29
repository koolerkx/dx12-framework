#include "bmfont_parser.h"

#include <algorithm>
#include <fstream>
#include <sstream>

namespace BMFont {

namespace {

// Helper: Remove quotes from string
std::string RemoveQuotes(const std::string& str) {
  if (str.size() >= 2 && str.front() == '"' && str.back() == '"') {
    return str.substr(1, str.size() - 2);
  }
  return str;
}

// Helper: Parse key=value pair
bool ParseKeyValue(const std::string& token, std::string& key, std::string& value) {
  size_t eq_pos = token.find('=');
  if (eq_pos == std::string::npos) {
    return false;
  }
  key = token.substr(0, eq_pos);
  value = token.substr(eq_pos + 1);
  value = RemoveQuotes(value);
  return true;
}

// Parse info line
void ParseInfo(std::istringstream& ss, BmFontInfo& info) {
  std::string token, key, value;
  while (ss >> token) {
    if (!ParseKeyValue(token, key, value)) continue;

    if (key == "face")
      info.face = value;
    else if (key == "size")
      info.size = std::stoi(value);
    else if (key == "bold")
      info.bold = (std::stoi(value) != 0);
    else if (key == "italic")
      info.italic = (std::stoi(value) != 0);
    else if (key == "charset")
      info.charset = value;
    else if (key == "unicode")
      info.unicode = (std::stoi(value) != 0);
    else if (key == "stretchH")
      info.stretchH = std::stoi(value);
    else if (key == "smooth")
      info.smooth = (std::stoi(value) != 0);
    else if (key == "aa")
      info.aa = std::stoi(value);
    else if (key == "outline")
      info.outline = std::stoi(value);
    else if (key == "padding") {
      // Format: "0,0,0,0"
      std::replace(value.begin(), value.end(), ',', ' ');
      std::istringstream ps(value);
      for (int i = 0; i < 4 && ps >> info.padding[i]; ++i)
        ;
    } else if (key == "spacing") {
      std::replace(value.begin(), value.end(), ',', ' ');
      std::istringstream ps(value);
      for (int i = 0; i < 2 && ps >> info.spacing[i]; ++i)
        ;
    }
  }
}

// Parse common line
void ParseCommon(std::istringstream& ss, BmFontCommon& common) {
  std::string token, key, value;
  while (ss >> token) {
    if (!ParseKeyValue(token, key, value)) continue;

    if (key == "lineHeight")
      common.lineHeight = static_cast<uint16_t>(std::stoul(value));
    else if (key == "base")
      common.base = static_cast<uint16_t>(std::stoul(value));
    else if (key == "scaleW")
      common.scaleW = static_cast<uint16_t>(std::stoul(value));
    else if (key == "scaleH")
      common.scaleH = static_cast<uint16_t>(std::stoul(value));
    else if (key == "pages")
      common.pages = static_cast<uint16_t>(std::stoul(value));
    else if (key == "packed")
      common.packed = (std::stoi(value) != 0);
    else if (key == "alphaChnl")
      common.alphaChnl = static_cast<uint8_t>(std::stoul(value));
    else if (key == "redChnl")
      common.redChnl = static_cast<uint8_t>(std::stoul(value));
    else if (key == "greenChnl")
      common.greenChnl = static_cast<uint8_t>(std::stoul(value));
    else if (key == "blueChnl")
      common.blueChnl = static_cast<uint8_t>(std::stoul(value));
  }
}

// Parse page line
void ParsePage(std::istringstream& ss, std::vector<BmFontPage>& pages) {
  BmFontPage page;
  std::string token, key, value;
  while (ss >> token) {
    if (!ParseKeyValue(token, key, value)) continue;

    if (key == "id")
      page.id = static_cast<uint16_t>(std::stoul(value));
    else if (key == "file")
      page.file = value;
  }
  pages.push_back(page);
}

// Parse char line
void ParseChar(std::istringstream& ss, std::unordered_map<uint32_t, BmFontChar>& chars) {
  BmFontChar c;
  std::string token, key, value;
  while (ss >> token) {
    if (!ParseKeyValue(token, key, value)) continue;

    if (key == "id")
      c.id = static_cast<uint32_t>(std::stoul(value));
    else if (key == "x")
      c.x = static_cast<uint16_t>(std::stoul(value));
    else if (key == "y")
      c.y = static_cast<uint16_t>(std::stoul(value));
    else if (key == "width")
      c.width = static_cast<uint16_t>(std::stoul(value));
    else if (key == "height")
      c.height = static_cast<uint16_t>(std::stoul(value));
    else if (key == "xoffset")
      c.xoffset = static_cast<int16_t>(std::stoi(value));
    else if (key == "yoffset")
      c.yoffset = static_cast<int16_t>(std::stoi(value));
    else if (key == "xadvance")
      c.xadvance = static_cast<int16_t>(std::stoi(value));
    else if (key == "page")
      c.page = static_cast<uint8_t>(std::stoul(value));
    else if (key == "chnl")
      c.chnl = static_cast<uint8_t>(std::stoul(value));
  }
  chars[c.id] = c;
}

// Parse kerning line
void ParseKerning(std::istringstream& ss, std::unordered_map<uint64_t, int16_t>& kernings) {
  BmFontKerning kern;
  std::string token, key, value;
  while (ss >> token) {
    if (!ParseKeyValue(token, key, value)) continue;

    if (key == "first")
      kern.first = static_cast<uint32_t>(std::stoul(value));
    else if (key == "second")
      kern.second = static_cast<uint32_t>(std::stoul(value));
    else if (key == "amount")
      kern.amount = static_cast<int16_t>(std::stoi(value));
  }
  uint64_t kernings_key = (static_cast<uint64_t>(kern.first) << 32) | kern.second;
  kernings[kernings_key] = kern.amount;
}

}  // namespace

bool ParseBmFontText(const std::string& filepath, BmFontData& out_data) {
  std::ifstream file(filepath);
  if (!file.is_open()) {
    return false;
  }

  std::string line;
  while (std::getline(file, line)) {
    if (line.empty()) continue;

    std::istringstream ss(line);
    std::string tag;
    ss >> tag;

    if (tag == "info") {
      ParseInfo(ss, out_data.info);
    } else if (tag == "common") {
      ParseCommon(ss, out_data.common);
    } else if (tag == "page") {
      ParsePage(ss, out_data.pages);
    } else if (tag == "char") {
      ParseChar(ss, out_data.chars);
    } else if (tag == "kerning") {
      ParseKerning(ss, out_data.kernings);
    }
    // "chars count=..." line is just informational, ignore
  }

  return true;
}

}  // namespace bmfont
