#pragma once

#include <string_view>

#include "Framework/Math/Math.h"


using Color = Math::Vector4;

namespace colors {

constexpr float HexCharToValue(char c) {
  if (c >= '0' && c <= '9') return static_cast<float>(c - '0');
  if (c >= 'A' && c <= 'F') return static_cast<float>(c - 'A' + 10);
  if (c >= 'a' && c <= 'f') return static_cast<float>(c - 'a' + 10);
  return 0.0f;
}

constexpr float HexPairToFloat(char hi, char lo) {
  return (HexCharToValue(hi) * 16.0f + HexCharToValue(lo)) / 255.0f;
}

inline Color ColorFromHex(std::string_view hex, float alpha = 1.0f) {
  return Color(HexPairToFloat(hex[1], hex[2]), HexPairToFloat(hex[3], hex[4]), HexPairToFloat(hex[5], hex[6]), alpha);
}

inline Color WithAlpha(Color c, float alpha) {
  return Color(c.x, c.y, c.z, alpha);
}

// Transparent
[[maybe_unused]] inline const Color TransparentWhite = ColorFromHex("#FFFFFF", 0.0f);
[[maybe_unused]] inline const Color TransparentBlack = ColorFromHex("#000000", 0.0f);

// HTML Colors (https://htmlcolorcodes.com/color-names/)
// Red
[[maybe_unused]] inline const Color IndianRed = ColorFromHex("#CD5C5C");
[[maybe_unused]] inline const Color LightCoral = ColorFromHex("#F08080");
[[maybe_unused]] inline const Color Salmon = ColorFromHex("#FA8072");
[[maybe_unused]] inline const Color DarkSalmon = ColorFromHex("#E9967A");
[[maybe_unused]] inline const Color LightSalmon = ColorFromHex("#FFA07A");
[[maybe_unused]] inline const Color Crimson = ColorFromHex("#DC143C");
[[maybe_unused]] inline const Color Red = ColorFromHex("#FF0000");
[[maybe_unused]] inline const Color FireBrick = ColorFromHex("#B22222");
[[maybe_unused]] inline const Color DarkRed = ColorFromHex("#8B0000");

// Pink
[[maybe_unused]] inline const Color Pink = ColorFromHex("#FFC0CB");
[[maybe_unused]] inline const Color LightPink = ColorFromHex("#FFB6C1");
[[maybe_unused]] inline const Color HotPink = ColorFromHex("#FF69B4");
[[maybe_unused]] inline const Color DeepPink = ColorFromHex("#FF1493");
[[maybe_unused]] inline const Color MediumVioletRed = ColorFromHex("#C71585");
[[maybe_unused]] inline const Color PaleVioletRed = ColorFromHex("#DB7093");

// Orange
[[maybe_unused]] inline const Color Coral = ColorFromHex("#FF7F50");
[[maybe_unused]] inline const Color Tomato = ColorFromHex("#FF6347");
[[maybe_unused]] inline const Color OrangeRed = ColorFromHex("#FF4500");
[[maybe_unused]] inline const Color DarkOrange = ColorFromHex("#FF8C00");
[[maybe_unused]] inline const Color Orange = ColorFromHex("#FFA500");

// Yellow
[[maybe_unused]] inline const Color Gold = ColorFromHex("#FFD700");
[[maybe_unused]] inline const Color Yellow = ColorFromHex("#FFFF00");
[[maybe_unused]] inline const Color LightYellow = ColorFromHex("#FFFFE0");
[[maybe_unused]] inline const Color LemonChiffon = ColorFromHex("#FFFACD");
[[maybe_unused]] inline const Color LightGoldenrodYellow = ColorFromHex("#FAFAD2");
[[maybe_unused]] inline const Color PapayaWhip = ColorFromHex("#FFEFD5");
[[maybe_unused]] inline const Color Moccasin = ColorFromHex("#FFE4B5");
[[maybe_unused]] inline const Color PeachPuff = ColorFromHex("#FFDAB9");
[[maybe_unused]] inline const Color PaleGoldenrod = ColorFromHex("#EEE8AA");
[[maybe_unused]] inline const Color Khaki = ColorFromHex("#F0E68C");
[[maybe_unused]] inline const Color DarkKhaki = ColorFromHex("#BDB76B");

// Purple
[[maybe_unused]] inline const Color Lavender = ColorFromHex("#E6E6FA");
[[maybe_unused]] inline const Color Thistle = ColorFromHex("#D8BFD8");
[[maybe_unused]] inline const Color Plum = ColorFromHex("#DDA0DD");
[[maybe_unused]] inline const Color Violet = ColorFromHex("#EE82EE");
[[maybe_unused]] inline const Color Orchid = ColorFromHex("#DA70D6");
[[maybe_unused]] inline const Color Fuchsia = ColorFromHex("#FF00FF");
[[maybe_unused]] inline const Color Magenta = ColorFromHex("#FF00FF");
[[maybe_unused]] inline const Color MediumOrchid = ColorFromHex("#BA55D3");
[[maybe_unused]] inline const Color MediumPurple = ColorFromHex("#9370DB");
[[maybe_unused]] inline const Color RebeccaPurple = ColorFromHex("#663399");
[[maybe_unused]] inline const Color BlueViolet = ColorFromHex("#8A2BE2");
[[maybe_unused]] inline const Color DarkViolet = ColorFromHex("#9400D3");
[[maybe_unused]] inline const Color DarkOrchid = ColorFromHex("#9932CC");
[[maybe_unused]] inline const Color DarkMagenta = ColorFromHex("#8B008B");
[[maybe_unused]] inline const Color Purple = ColorFromHex("#800080");
[[maybe_unused]] inline const Color Indigo = ColorFromHex("#4B0082");
[[maybe_unused]] inline const Color SlateBlue = ColorFromHex("#6A5ACD");
[[maybe_unused]] inline const Color DarkSlateBlue = ColorFromHex("#483D8B");
[[maybe_unused]] inline const Color MediumSlateBlue = ColorFromHex("#7B68EE");

// Green
[[maybe_unused]] inline const Color GreenYellow = ColorFromHex("#ADFF2F");
[[maybe_unused]] inline const Color Chartreuse = ColorFromHex("#7FFF00");
[[maybe_unused]] inline const Color LawnGreen = ColorFromHex("#7CFC00");
[[maybe_unused]] inline const Color Lime = ColorFromHex("#00FF00");
[[maybe_unused]] inline const Color LimeGreen = ColorFromHex("#32CD32");
[[maybe_unused]] inline const Color PaleGreen = ColorFromHex("#98FB98");
[[maybe_unused]] inline const Color LightGreen = ColorFromHex("#90EE90");
[[maybe_unused]] inline const Color MediumSpringGreen = ColorFromHex("#00FA9A");
[[maybe_unused]] inline const Color SpringGreen = ColorFromHex("#00FF7F");
[[maybe_unused]] inline const Color MediumSeaGreen = ColorFromHex("#3CB371");
[[maybe_unused]] inline const Color SeaGreen = ColorFromHex("#2E8B57");
[[maybe_unused]] inline const Color ForestGreen = ColorFromHex("#228B22");
[[maybe_unused]] inline const Color Green = ColorFromHex("#008000");
[[maybe_unused]] inline const Color DarkGreen = ColorFromHex("#006400");
[[maybe_unused]] inline const Color YellowGreen = ColorFromHex("#9ACD32");
[[maybe_unused]] inline const Color OliveDrab = ColorFromHex("#6B8E23");
[[maybe_unused]] inline const Color Olive = ColorFromHex("#808000");
[[maybe_unused]] inline const Color DarkOliveGreen = ColorFromHex("#556B2F");
[[maybe_unused]] inline const Color MediumAquamarine = ColorFromHex("#66CDAA");
[[maybe_unused]] inline const Color DarkSeaGreen = ColorFromHex("#8FBC8B");
[[maybe_unused]] inline const Color LightSeaGreen = ColorFromHex("#20B2AA");
[[maybe_unused]] inline const Color DarkCyan = ColorFromHex("#008B8B");
[[maybe_unused]] inline const Color Teal = ColorFromHex("#008080");

// Blue
[[maybe_unused]] inline const Color Aqua = ColorFromHex("#00FFFF");
[[maybe_unused]] inline const Color Cyan = ColorFromHex("#00FFFF");
[[maybe_unused]] inline const Color LightCyan = ColorFromHex("#E0FFFF");
[[maybe_unused]] inline const Color PaleTurquoise = ColorFromHex("#AFEEEE");
[[maybe_unused]] inline const Color Aquamarine = ColorFromHex("#7FFFD4");
[[maybe_unused]] inline const Color Turquoise = ColorFromHex("#40E0D0");
[[maybe_unused]] inline const Color MediumTurquoise = ColorFromHex("#48D1CC");
[[maybe_unused]] inline const Color DarkTurquoise = ColorFromHex("#00CED1");
[[maybe_unused]] inline const Color CadetBlue = ColorFromHex("#5F9EA0");
[[maybe_unused]] inline const Color SteelBlue = ColorFromHex("#4682B4");
[[maybe_unused]] inline const Color LightSteelBlue = ColorFromHex("#B0C4DE");
[[maybe_unused]] inline const Color PowderBlue = ColorFromHex("#B0E0E6");
[[maybe_unused]] inline const Color LightBlue = ColorFromHex("#ADD8E6");
[[maybe_unused]] inline const Color SkyBlue = ColorFromHex("#87CEEB");
[[maybe_unused]] inline const Color LightSkyBlue = ColorFromHex("#87CEFA");
[[maybe_unused]] inline const Color DeepSkyBlue = ColorFromHex("#00BFFF");
[[maybe_unused]] inline const Color DodgerBlue = ColorFromHex("#1E90FF");
[[maybe_unused]] inline const Color CornflowerBlue = ColorFromHex("#6495ED");
[[maybe_unused]] inline const Color RoyalBlue = ColorFromHex("#4169E1");
[[maybe_unused]] inline const Color Blue = ColorFromHex("#0000FF");
[[maybe_unused]] inline const Color MediumBlue = ColorFromHex("#0000CD");
[[maybe_unused]] inline const Color DarkBlue = ColorFromHex("#00008B");
[[maybe_unused]] inline const Color Navy = ColorFromHex("#000080");
[[maybe_unused]] inline const Color MidnightBlue = ColorFromHex("#191970");

// Brown
[[maybe_unused]] inline const Color Cornsilk = ColorFromHex("#FFF8DC");
[[maybe_unused]] inline const Color BlanchedAlmond = ColorFromHex("#FFEBCD");
[[maybe_unused]] inline const Color Bisque = ColorFromHex("#FFE4C4");
[[maybe_unused]] inline const Color NavajoWhite = ColorFromHex("#FFDEAD");
[[maybe_unused]] inline const Color Wheat = ColorFromHex("#F5DEB3");
[[maybe_unused]] inline const Color BurlyWood = ColorFromHex("#DEB887");
[[maybe_unused]] inline const Color Tan = ColorFromHex("#D2B48C");
[[maybe_unused]] inline const Color RosyBrown = ColorFromHex("#BC8F8F");
[[maybe_unused]] inline const Color SandyBrown = ColorFromHex("#F4A460");
[[maybe_unused]] inline const Color Goldenrod = ColorFromHex("#DAA520");
[[maybe_unused]] inline const Color DarkGoldenrod = ColorFromHex("#B8860B");
[[maybe_unused]] inline const Color Peru = ColorFromHex("#CD853F");
[[maybe_unused]] inline const Color Chocolate = ColorFromHex("#D2691E");
[[maybe_unused]] inline const Color SaddleBrown = ColorFromHex("#8B4513");
[[maybe_unused]] inline const Color Sienna = ColorFromHex("#A0522D");
[[maybe_unused]] inline const Color Brown = ColorFromHex("#A52A2A");
[[maybe_unused]] inline const Color Maroon = ColorFromHex("#800000");

// White
[[maybe_unused]] inline const Color White = ColorFromHex("#FFFFFF");
[[maybe_unused]] inline const Color Snow = ColorFromHex("#FFFAFA");
[[maybe_unused]] inline const Color HoneyDew = ColorFromHex("#F0FFF0");
[[maybe_unused]] inline const Color MintCream = ColorFromHex("#F5FFFA");
[[maybe_unused]] inline const Color Azure = ColorFromHex("#F0FFFF");
[[maybe_unused]] inline const Color AliceBlue = ColorFromHex("#F0F8FF");
[[maybe_unused]] inline const Color GhostWhite = ColorFromHex("#F8F8FF");
[[maybe_unused]] inline const Color WhiteSmoke = ColorFromHex("#F5F5F5");
[[maybe_unused]] inline const Color SeaShell = ColorFromHex("#FFF5EE");
[[maybe_unused]] inline const Color Beige = ColorFromHex("#F5F5DC");
[[maybe_unused]] inline const Color OldLace = ColorFromHex("#FDF5E6");
[[maybe_unused]] inline const Color FloralWhite = ColorFromHex("#FFFAF0");
[[maybe_unused]] inline const Color Ivory = ColorFromHex("#FFFFF0");
[[maybe_unused]] inline const Color AntiqueWhite = ColorFromHex("#FAEBD7");
[[maybe_unused]] inline const Color Linen = ColorFromHex("#FAF0E6");
[[maybe_unused]] inline const Color LavenderBlush = ColorFromHex("#FFF0F5");
[[maybe_unused]] inline const Color MistyRose = ColorFromHex("#FFE4E1");

// Gray
[[maybe_unused]] inline const Color Gainsboro = ColorFromHex("#DCDCDC");
[[maybe_unused]] inline const Color LightGray = ColorFromHex("#D3D3D3");
[[maybe_unused]] inline const Color Silver = ColorFromHex("#C0C0C0");
[[maybe_unused]] inline const Color DarkGray = ColorFromHex("#A9A9A9");
[[maybe_unused]] inline const Color Gray = ColorFromHex("#808080");
[[maybe_unused]] inline const Color DimGray = ColorFromHex("#696969");
[[maybe_unused]] inline const Color LightSlateGray = ColorFromHex("#778899");
[[maybe_unused]] inline const Color SlateGray = ColorFromHex("#708090");
[[maybe_unused]] inline const Color DarkSlateGray = ColorFromHex("#2F4F4F");
[[maybe_unused]] inline const Color Black = ColorFromHex("#000000");

// ---------------------------------------------------------------------------
// Material Design 2 Colors (https://m2.material.io/design/color)
// ---------------------------------------------------------------------------

// Red
[[maybe_unused]] inline const Color Red50 = ColorFromHex("#FFEBEE");
[[maybe_unused]] inline const Color Red100 = ColorFromHex("#FFCDD2");
[[maybe_unused]] inline const Color Red200 = ColorFromHex("#EF9A9A");
[[maybe_unused]] inline const Color Red300 = ColorFromHex("#E57373");
[[maybe_unused]] inline const Color Red400 = ColorFromHex("#EF5350");
[[maybe_unused]] inline const Color Red500 = ColorFromHex("#F44336");
[[maybe_unused]] inline const Color Red600 = ColorFromHex("#E53935");
[[maybe_unused]] inline const Color Red700 = ColorFromHex("#D32F2F");
[[maybe_unused]] inline const Color Red800 = ColorFromHex("#C62828");
[[maybe_unused]] inline const Color Red900 = ColorFromHex("#B71C1C");
[[maybe_unused]] inline const Color RedA100 = ColorFromHex("#FF8A80");
[[maybe_unused]] inline const Color RedA200 = ColorFromHex("#FF5252");
[[maybe_unused]] inline const Color RedA400 = ColorFromHex("#FF1744");
[[maybe_unused]] inline const Color RedA700 = ColorFromHex("#D50000");

// Pink
[[maybe_unused]] inline const Color Pink50 = ColorFromHex("#FCE4EC");
[[maybe_unused]] inline const Color Pink100 = ColorFromHex("#F8BBD0");
[[maybe_unused]] inline const Color Pink200 = ColorFromHex("#F48FB1");
[[maybe_unused]] inline const Color Pink300 = ColorFromHex("#F06292");
[[maybe_unused]] inline const Color Pink400 = ColorFromHex("#EC407A");
[[maybe_unused]] inline const Color Pink500 = ColorFromHex("#E91E63");
[[maybe_unused]] inline const Color Pink600 = ColorFromHex("#D81B60");
[[maybe_unused]] inline const Color Pink700 = ColorFromHex("#C2185B");
[[maybe_unused]] inline const Color Pink800 = ColorFromHex("#AD1457");
[[maybe_unused]] inline const Color Pink900 = ColorFromHex("#880E4F");
[[maybe_unused]] inline const Color PinkA100 = ColorFromHex("#FF80AB");
[[maybe_unused]] inline const Color PinkA200 = ColorFromHex("#FF4081");
[[maybe_unused]] inline const Color PinkA400 = ColorFromHex("#F50057");
[[maybe_unused]] inline const Color PinkA700 = ColorFromHex("#C51162");

// Purple
[[maybe_unused]] inline const Color Purple50 = ColorFromHex("#F3E5F5");
[[maybe_unused]] inline const Color Purple100 = ColorFromHex("#E1BEE7");
[[maybe_unused]] inline const Color Purple200 = ColorFromHex("#CE93D8");
[[maybe_unused]] inline const Color Purple300 = ColorFromHex("#BA68C8");
[[maybe_unused]] inline const Color Purple400 = ColorFromHex("#AB47BC");
[[maybe_unused]] inline const Color Purple500 = ColorFromHex("#9C27B0");
[[maybe_unused]] inline const Color Purple600 = ColorFromHex("#8E24AA");
[[maybe_unused]] inline const Color Purple700 = ColorFromHex("#7B1FA2");
[[maybe_unused]] inline const Color Purple800 = ColorFromHex("#6A1B9A");
[[maybe_unused]] inline const Color Purple900 = ColorFromHex("#4A148C");
[[maybe_unused]] inline const Color PurpleA100 = ColorFromHex("#EA80FC");
[[maybe_unused]] inline const Color PurpleA200 = ColorFromHex("#E040FB");
[[maybe_unused]] inline const Color PurpleA400 = ColorFromHex("#D500F9");
[[maybe_unused]] inline const Color PurpleA700 = ColorFromHex("#AA00FF");

// Deep Purple
[[maybe_unused]] inline const Color DeepPurple50 = ColorFromHex("#EDE7F6");
[[maybe_unused]] inline const Color DeepPurple100 = ColorFromHex("#D1C4E9");
[[maybe_unused]] inline const Color DeepPurple200 = ColorFromHex("#B39DDB");
[[maybe_unused]] inline const Color DeepPurple300 = ColorFromHex("#9575CD");
[[maybe_unused]] inline const Color DeepPurple400 = ColorFromHex("#7E57C2");
[[maybe_unused]] inline const Color DeepPurple500 = ColorFromHex("#673AB7");
[[maybe_unused]] inline const Color DeepPurple600 = ColorFromHex("#5E35B1");
[[maybe_unused]] inline const Color DeepPurple700 = ColorFromHex("#512DA8");
[[maybe_unused]] inline const Color DeepPurple800 = ColorFromHex("#4527A0");
[[maybe_unused]] inline const Color DeepPurple900 = ColorFromHex("#311B92");
[[maybe_unused]] inline const Color DeepPurpleA100 = ColorFromHex("#B388FF");
[[maybe_unused]] inline const Color DeepPurpleA200 = ColorFromHex("#7C4DFF");
[[maybe_unused]] inline const Color DeepPurpleA400 = ColorFromHex("#651FFF");
[[maybe_unused]] inline const Color DeepPurpleA700 = ColorFromHex("#6200EA");

// Indigo
[[maybe_unused]] inline const Color Indigo50 = ColorFromHex("#E8EAF6");
[[maybe_unused]] inline const Color Indigo100 = ColorFromHex("#C5CAE9");
[[maybe_unused]] inline const Color Indigo200 = ColorFromHex("#9FA8DA");
[[maybe_unused]] inline const Color Indigo300 = ColorFromHex("#7986CB");
[[maybe_unused]] inline const Color Indigo400 = ColorFromHex("#5C6BC0");
[[maybe_unused]] inline const Color Indigo500 = ColorFromHex("#3F51B5");
[[maybe_unused]] inline const Color Indigo600 = ColorFromHex("#3949AB");
[[maybe_unused]] inline const Color Indigo700 = ColorFromHex("#303F9F");
[[maybe_unused]] inline const Color Indigo800 = ColorFromHex("#283593");
[[maybe_unused]] inline const Color Indigo900 = ColorFromHex("#1A237E");
[[maybe_unused]] inline const Color IndigoA100 = ColorFromHex("#8C9EFF");
[[maybe_unused]] inline const Color IndigoA200 = ColorFromHex("#536DFE");
[[maybe_unused]] inline const Color IndigoA400 = ColorFromHex("#3D5AFE");
[[maybe_unused]] inline const Color IndigoA700 = ColorFromHex("#304FFE");

// Blue
[[maybe_unused]] inline const Color Blue50 = ColorFromHex("#E3F2FD");
[[maybe_unused]] inline const Color Blue100 = ColorFromHex("#BBDEFB");
[[maybe_unused]] inline const Color Blue200 = ColorFromHex("#90CAF9");
[[maybe_unused]] inline const Color Blue300 = ColorFromHex("#64B5F6");
[[maybe_unused]] inline const Color Blue400 = ColorFromHex("#42A5F5");
[[maybe_unused]] inline const Color Blue500 = ColorFromHex("#2196F3");
[[maybe_unused]] inline const Color Blue600 = ColorFromHex("#1E88E5");
[[maybe_unused]] inline const Color Blue700 = ColorFromHex("#1976D2");
[[maybe_unused]] inline const Color Blue800 = ColorFromHex("#1565C0");
[[maybe_unused]] inline const Color Blue900 = ColorFromHex("#0D47A1");
[[maybe_unused]] inline const Color BlueA100 = ColorFromHex("#82B1FF");
[[maybe_unused]] inline const Color BlueA200 = ColorFromHex("#448AFF");
[[maybe_unused]] inline const Color BlueA400 = ColorFromHex("#2979FF");
[[maybe_unused]] inline const Color BlueA700 = ColorFromHex("#2962FF");

// Light Blue
[[maybe_unused]] inline const Color LightBlue50 = ColorFromHex("#E1F5FE");
[[maybe_unused]] inline const Color LightBlue100 = ColorFromHex("#B3E5FC");
[[maybe_unused]] inline const Color LightBlue200 = ColorFromHex("#81D4FA");
[[maybe_unused]] inline const Color LightBlue300 = ColorFromHex("#4FC3F7");
[[maybe_unused]] inline const Color LightBlue400 = ColorFromHex("#29B6F6");
[[maybe_unused]] inline const Color LightBlue500 = ColorFromHex("#03A9F4");
[[maybe_unused]] inline const Color LightBlue600 = ColorFromHex("#039BE5");
[[maybe_unused]] inline const Color LightBlue700 = ColorFromHex("#0288D1");
[[maybe_unused]] inline const Color LightBlue800 = ColorFromHex("#0277BD");
[[maybe_unused]] inline const Color LightBlue900 = ColorFromHex("#01579B");
[[maybe_unused]] inline const Color LightBlueA100 = ColorFromHex("#80D8FF");
[[maybe_unused]] inline const Color LightBlueA200 = ColorFromHex("#40C4FF");
[[maybe_unused]] inline const Color LightBlueA400 = ColorFromHex("#00B0FF");
[[maybe_unused]] inline const Color LightBlueA700 = ColorFromHex("#0091EA");

// Cyan
[[maybe_unused]] inline const Color Cyan50 = ColorFromHex("#E0F7FA");
[[maybe_unused]] inline const Color Cyan100 = ColorFromHex("#B2EBF2");
[[maybe_unused]] inline const Color Cyan200 = ColorFromHex("#80DEEA");
[[maybe_unused]] inline const Color Cyan300 = ColorFromHex("#4DD0E1");
[[maybe_unused]] inline const Color Cyan400 = ColorFromHex("#26C6DA");
[[maybe_unused]] inline const Color Cyan500 = ColorFromHex("#00BCD4");
[[maybe_unused]] inline const Color Cyan600 = ColorFromHex("#00ACC1");
[[maybe_unused]] inline const Color Cyan700 = ColorFromHex("#0097A7");
[[maybe_unused]] inline const Color Cyan800 = ColorFromHex("#00838F");
[[maybe_unused]] inline const Color Cyan900 = ColorFromHex("#006064");
[[maybe_unused]] inline const Color CyanA100 = ColorFromHex("#84FFFF");
[[maybe_unused]] inline const Color CyanA200 = ColorFromHex("#18FFFF");
[[maybe_unused]] inline const Color CyanA400 = ColorFromHex("#00E5FF");
[[maybe_unused]] inline const Color CyanA700 = ColorFromHex("#00B8D4");

// Teal
[[maybe_unused]] inline const Color Teal50 = ColorFromHex("#E0F2F1");
[[maybe_unused]] inline const Color Teal100 = ColorFromHex("#B2DFDB");
[[maybe_unused]] inline const Color Teal200 = ColorFromHex("#80CBC4");
[[maybe_unused]] inline const Color Teal300 = ColorFromHex("#4DB6AC");
[[maybe_unused]] inline const Color Teal400 = ColorFromHex("#26A69A");
[[maybe_unused]] inline const Color Teal500 = ColorFromHex("#009688");
[[maybe_unused]] inline const Color Teal600 = ColorFromHex("#00897B");
[[maybe_unused]] inline const Color Teal700 = ColorFromHex("#00796B");
[[maybe_unused]] inline const Color Teal800 = ColorFromHex("#00695C");
[[maybe_unused]] inline const Color Teal900 = ColorFromHex("#004D40");
[[maybe_unused]] inline const Color TealA100 = ColorFromHex("#A7FFEB");
[[maybe_unused]] inline const Color TealA200 = ColorFromHex("#64FFDA");
[[maybe_unused]] inline const Color TealA400 = ColorFromHex("#1DE9B6");
[[maybe_unused]] inline const Color TealA700 = ColorFromHex("#00BFA5");

// Green
[[maybe_unused]] inline const Color Green50 = ColorFromHex("#E8F5E9");
[[maybe_unused]] inline const Color Green100 = ColorFromHex("#C8E6C9");
[[maybe_unused]] inline const Color Green200 = ColorFromHex("#A5D6A7");
[[maybe_unused]] inline const Color Green300 = ColorFromHex("#81C784");
[[maybe_unused]] inline const Color Green400 = ColorFromHex("#66BB6A");
[[maybe_unused]] inline const Color Green500 = ColorFromHex("#4CAF50");
[[maybe_unused]] inline const Color Green600 = ColorFromHex("#43A047");
[[maybe_unused]] inline const Color Green700 = ColorFromHex("#388E3C");
[[maybe_unused]] inline const Color Green800 = ColorFromHex("#2E7D32");
[[maybe_unused]] inline const Color Green900 = ColorFromHex("#1B5E20");
[[maybe_unused]] inline const Color GreenA100 = ColorFromHex("#B9F6CA");
[[maybe_unused]] inline const Color GreenA200 = ColorFromHex("#69F0AE");
[[maybe_unused]] inline const Color GreenA400 = ColorFromHex("#00E676");
[[maybe_unused]] inline const Color GreenA700 = ColorFromHex("#00C853");

// Light Green
[[maybe_unused]] inline const Color LightGreen50 = ColorFromHex("#F1F8E9");
[[maybe_unused]] inline const Color LightGreen100 = ColorFromHex("#DCEDC8");
[[maybe_unused]] inline const Color LightGreen200 = ColorFromHex("#C5E1A5");
[[maybe_unused]] inline const Color LightGreen300 = ColorFromHex("#AED581");
[[maybe_unused]] inline const Color LightGreen400 = ColorFromHex("#9CCC65");
[[maybe_unused]] inline const Color LightGreen500 = ColorFromHex("#8BC34A");
[[maybe_unused]] inline const Color LightGreen600 = ColorFromHex("#7CB342");
[[maybe_unused]] inline const Color LightGreen700 = ColorFromHex("#689F38");
[[maybe_unused]] inline const Color LightGreen800 = ColorFromHex("#558B2F");
[[maybe_unused]] inline const Color LightGreen900 = ColorFromHex("#33691E");
[[maybe_unused]] inline const Color LightGreenA100 = ColorFromHex("#CCFF90");
[[maybe_unused]] inline const Color LightGreenA200 = ColorFromHex("#B2FF59");
[[maybe_unused]] inline const Color LightGreenA400 = ColorFromHex("#76FF03");
[[maybe_unused]] inline const Color LightGreenA700 = ColorFromHex("#64DD17");

// Lime
[[maybe_unused]] inline const Color Lime50 = ColorFromHex("#F9FBE7");
[[maybe_unused]] inline const Color Lime100 = ColorFromHex("#F0F4C3");
[[maybe_unused]] inline const Color Lime200 = ColorFromHex("#E6EE9C");
[[maybe_unused]] inline const Color Lime300 = ColorFromHex("#DCE775");
[[maybe_unused]] inline const Color Lime400 = ColorFromHex("#D4E157");
[[maybe_unused]] inline const Color Lime500 = ColorFromHex("#CDDC39");
[[maybe_unused]] inline const Color Lime600 = ColorFromHex("#C0CA33");
[[maybe_unused]] inline const Color Lime700 = ColorFromHex("#AFB42B");
[[maybe_unused]] inline const Color Lime800 = ColorFromHex("#9E9D24");
[[maybe_unused]] inline const Color Lime900 = ColorFromHex("#827717");
[[maybe_unused]] inline const Color LimeA100 = ColorFromHex("#F4FF81");
[[maybe_unused]] inline const Color LimeA200 = ColorFromHex("#EEFF41");
[[maybe_unused]] inline const Color LimeA400 = ColorFromHex("#C6FF00");
[[maybe_unused]] inline const Color LimeA700 = ColorFromHex("#AEEA00");

// Yellow
[[maybe_unused]] inline const Color Yellow50 = ColorFromHex("#FFFDE7");
[[maybe_unused]] inline const Color Yellow100 = ColorFromHex("#FFF9C4");
[[maybe_unused]] inline const Color Yellow200 = ColorFromHex("#FFF59D");
[[maybe_unused]] inline const Color Yellow300 = ColorFromHex("#FFF176");
[[maybe_unused]] inline const Color Yellow400 = ColorFromHex("#FFEE58");
[[maybe_unused]] inline const Color Yellow500 = ColorFromHex("#FFEB3B");
[[maybe_unused]] inline const Color Yellow600 = ColorFromHex("#FDD835");
[[maybe_unused]] inline const Color Yellow700 = ColorFromHex("#FBC02D");
[[maybe_unused]] inline const Color Yellow800 = ColorFromHex("#F9A825");
[[maybe_unused]] inline const Color Yellow900 = ColorFromHex("#F57F17");
[[maybe_unused]] inline const Color YellowA100 = ColorFromHex("#FFFF8D");
[[maybe_unused]] inline const Color YellowA200 = ColorFromHex("#FFFF00");
[[maybe_unused]] inline const Color YellowA400 = ColorFromHex("#FFEA00");
[[maybe_unused]] inline const Color YellowA700 = ColorFromHex("#FFD600");

// Amber
[[maybe_unused]] inline const Color Amber50 = ColorFromHex("#FFF8E1");
[[maybe_unused]] inline const Color Amber100 = ColorFromHex("#FFECB3");
[[maybe_unused]] inline const Color Amber200 = ColorFromHex("#FFE082");
[[maybe_unused]] inline const Color Amber300 = ColorFromHex("#FFD54F");
[[maybe_unused]] inline const Color Amber400 = ColorFromHex("#FFCA28");
[[maybe_unused]] inline const Color Amber500 = ColorFromHex("#FFC107");
[[maybe_unused]] inline const Color Amber600 = ColorFromHex("#FFB300");
[[maybe_unused]] inline const Color Amber700 = ColorFromHex("#FFA000");
[[maybe_unused]] inline const Color Amber800 = ColorFromHex("#FF8F00");
[[maybe_unused]] inline const Color Amber900 = ColorFromHex("#FF6F00");
[[maybe_unused]] inline const Color AmberA100 = ColorFromHex("#FFE57F");
[[maybe_unused]] inline const Color AmberA200 = ColorFromHex("#FFD740");
[[maybe_unused]] inline const Color AmberA400 = ColorFromHex("#FFC400");
[[maybe_unused]] inline const Color AmberA700 = ColorFromHex("#FFAB00");

// Orange
[[maybe_unused]] inline const Color Orange50 = ColorFromHex("#FFF3E0");
[[maybe_unused]] inline const Color Orange100 = ColorFromHex("#FFE0B2");
[[maybe_unused]] inline const Color Orange200 = ColorFromHex("#FFCC80");
[[maybe_unused]] inline const Color Orange300 = ColorFromHex("#FFB74D");
[[maybe_unused]] inline const Color Orange400 = ColorFromHex("#FFA726");
[[maybe_unused]] inline const Color Orange500 = ColorFromHex("#FF9800");
[[maybe_unused]] inline const Color Orange600 = ColorFromHex("#FB8C00");
[[maybe_unused]] inline const Color Orange700 = ColorFromHex("#F57C00");
[[maybe_unused]] inline const Color Orange800 = ColorFromHex("#EF6C00");
[[maybe_unused]] inline const Color Orange900 = ColorFromHex("#E65100");
[[maybe_unused]] inline const Color OrangeA100 = ColorFromHex("#FFD180");
[[maybe_unused]] inline const Color OrangeA200 = ColorFromHex("#FFAB40");
[[maybe_unused]] inline const Color OrangeA400 = ColorFromHex("#FF9100");
[[maybe_unused]] inline const Color OrangeA700 = ColorFromHex("#FF6D00");

// Deep Orange
[[maybe_unused]] inline const Color DeepOrange50 = ColorFromHex("#FBE9E7");
[[maybe_unused]] inline const Color DeepOrange100 = ColorFromHex("#FFCCBC");
[[maybe_unused]] inline const Color DeepOrange200 = ColorFromHex("#FFAB91");
[[maybe_unused]] inline const Color DeepOrange300 = ColorFromHex("#FF8A65");
[[maybe_unused]] inline const Color DeepOrange400 = ColorFromHex("#FF7043");
[[maybe_unused]] inline const Color DeepOrange500 = ColorFromHex("#FF5722");
[[maybe_unused]] inline const Color DeepOrange600 = ColorFromHex("#F4511E");
[[maybe_unused]] inline const Color DeepOrange700 = ColorFromHex("#E64A19");
[[maybe_unused]] inline const Color DeepOrange800 = ColorFromHex("#D84315");
[[maybe_unused]] inline const Color DeepOrange900 = ColorFromHex("#BF360C");
[[maybe_unused]] inline const Color DeepOrangeA100 = ColorFromHex("#FF9E80");
[[maybe_unused]] inline const Color DeepOrangeA200 = ColorFromHex("#FF6E40");
[[maybe_unused]] inline const Color DeepOrangeA400 = ColorFromHex("#FF3D00");
[[maybe_unused]] inline const Color DeepOrangeA700 = ColorFromHex("#DD2C00");

// Brown
[[maybe_unused]] inline const Color Brown50 = ColorFromHex("#EFEBE9");
[[maybe_unused]] inline const Color Brown100 = ColorFromHex("#D7CCC8");
[[maybe_unused]] inline const Color Brown200 = ColorFromHex("#BCAAA4");
[[maybe_unused]] inline const Color Brown300 = ColorFromHex("#A1887F");
[[maybe_unused]] inline const Color Brown400 = ColorFromHex("#8D6E63");
[[maybe_unused]] inline const Color Brown500 = ColorFromHex("#795548");
[[maybe_unused]] inline const Color Brown600 = ColorFromHex("#6D4C41");
[[maybe_unused]] inline const Color Brown700 = ColorFromHex("#5D4037");
[[maybe_unused]] inline const Color Brown800 = ColorFromHex("#4E342E");
[[maybe_unused]] inline const Color Brown900 = ColorFromHex("#3E2723");

// Grey
[[maybe_unused]] inline const Color Grey50 = ColorFromHex("#FAFAFA");
[[maybe_unused]] inline const Color Grey100 = ColorFromHex("#F5F5F5");
[[maybe_unused]] inline const Color Grey200 = ColorFromHex("#EEEEEE");
[[maybe_unused]] inline const Color Grey300 = ColorFromHex("#E0E0E0");
[[maybe_unused]] inline const Color Grey400 = ColorFromHex("#BDBDBD");
[[maybe_unused]] inline const Color Grey500 = ColorFromHex("#9E9E9E");
[[maybe_unused]] inline const Color Grey600 = ColorFromHex("#757575");
[[maybe_unused]] inline const Color Grey700 = ColorFromHex("#616161");
[[maybe_unused]] inline const Color Grey800 = ColorFromHex("#424242");
[[maybe_unused]] inline const Color Grey900 = ColorFromHex("#212121");

// Blue Grey
[[maybe_unused]] inline const Color BlueGrey50 = ColorFromHex("#ECEFF1");
[[maybe_unused]] inline const Color BlueGrey100 = ColorFromHex("#CFD8DC");
[[maybe_unused]] inline const Color BlueGrey200 = ColorFromHex("#B0BEC5");
[[maybe_unused]] inline const Color BlueGrey300 = ColorFromHex("#90A4AE");
[[maybe_unused]] inline const Color BlueGrey400 = ColorFromHex("#78909C");
[[maybe_unused]] inline const Color BlueGrey500 = ColorFromHex("#607D8B");
[[maybe_unused]] inline const Color BlueGrey600 = ColorFromHex("#546E7A");
[[maybe_unused]] inline const Color BlueGrey700 = ColorFromHex("#455A64");
[[maybe_unused]] inline const Color BlueGrey800 = ColorFromHex("#37474F");
[[maybe_unused]] inline const Color BlueGrey900 = ColorFromHex("#263238");

}  // namespace colors
