#pragma once

#include <array>
#include <filesystem>
#include <vector>

namespace Wolf3dLoaders
{
struct Bitmap
{
    std::vector<uint8_t> data;
    int width{};
    int height{};
    int layers{};
};

enum class MapObjects
{
    PlayerNorth = 19,
    PlayerEast = 20,
    PlayerSouth = 21,
    PlayerWest = 22,
    GoldKey = 43,
    SilverKey = 44,
    Food = 47,
    FirstAidKit = 48,
    Ammo = 49,
    MachineGun = 50,
    Gatling = 51,
    Cross = 52,
    Chalice = 53,
    Jewels = 54,
    Crown = 55,
    ExtraLife = 56,
    SecretDoor = 98,
    EndGameTrigger = 99,
    EnemyFirst = 108
};

struct Map
{
    std::array<std::vector<uint16_t>, 2> tiles;
    int width{};
};

struct Enemy
{
    int baseIndex{0};
    float angle{0.0f};
    bool moving{false};
    int level{0};
};

constexpr Enemy Enemies[] = {
    {50, 0.0f, false, 1},   // 108 guard
    {50, 90.0f, false, 1},  // 109
    {50, 180.0f, false, 1}, // 110
    {50, 270.0f, false, 1}, // 111
    {50, 0.0f, true, 1},    // 112
    {50, 90.0f, true, 1},   // 113
    {50, 180.0f, true, 1},  // 114
    {50, 270.0f, true, 1},  // 115

    {238, 0.0f, false, 1},   // 116 officer
    {238, 90.0f, false, 1},  // 117
    {238, 180.0f, false, 1}, // 118
    {238, 270.0f, false, 1}, // 119
    {238, 0.0f, true, 1},    // 120
    {238, 90.0f, true, 1},   // 121
    {238, 180.0f, true, 1},  // 122
    {238, 270.0f, true, 1},  // 123

    {50, 270.0f, true, 1}, // 124 dead
    {50, 270.0f, true, 1}, // 125 none

    {138, 0.0f, false, 1},   // 126 blue
    {138, 90.0f, false, 1},  // 127
    {138, 180.0f, false, 1}, // 128
    {138, 270.0f, false, 1}, // 129
    {138, 0.0f, true, 1},    // 130
    {138, 90.0f, true, 1},   // 131
    {138, 180.0f, true, 1},  // 132
    {138, 270.0f, true, 1},  // 133

    {50, 270.0f, true, 1}, // 134 none
    {50, 270.0f, true, 1}, // 135 none
    {50, 270.0f, true, 1}, // 136 none
    {50, 270.0f, true, 1}, // 137 none

    {99, 0.0f, true, 1},   // 138 dog
    {99, 90.0f, true, 1},  // 139
    {99, 180.0f, true, 1}, // 140
    {99, 270.0f, true, 1}, // 141

    {99, 270.0f, true, 1}, // 142 none
    {99, 270.0f, true, 1}, // 143 none

    {50, 0.0f, false, 3},   // 144 guard
    {50, 90.0f, false, 3},  // 145
    {50, 180.0f, false, 3}, // 146
    {50, 270.0f, false, 3}, // 147
    {50, 0.0f, true, 3},    // 148
    {50, 90.0f, true, 3},   // 149
    {50, 180.0f, true, 3},  // 150
    {50, 270.0f, true, 3},  // 151

    {238, 0.0f, false, 3},   // 152 officer
    {238, 90.0f, false, 3},  // 153
    {238, 180.0f, false, 3}, // 154
    {238, 270.0f, false, 3}, // 155
    {238, 0.0f, true, 3},    // 156
    {238, 90.0f, true, 3},   // 157
    {238, 180.0f, true, 3},  // 158
    {238, 270.0f, true, 3},  // 159

    {238, 270.0f, true, 1}, // 160 boss

    {238, 270.0f, true, 1}, // 161 none

    {138, 0.0f, false, 3},   // 162 blue
    {138, 90.0f, false, 3},  // 163
    {138, 180.0f, false, 3}, // 164
    {138, 270.0f, false, 3}, // 165
    {138, 0.0f, true, 3},    // 166
    {138, 90.0f, true, 3},   // 167
    {138, 180.0f, true, 3},  // 168
    {138, 270.0f, true, 3},  // 169

    {99, 0.0f, true, 3},   // 174 dog
    {99, 90.0f, true, 3},  // 175
    {99, 180.0f, true, 3}, // 176
    {99, 270.0f, true, 3}, // 177

    {99, 270.0f, true, 1}, // 178 boss
    {99, 270.0f, true, 1}, // 179 boss

    {50, 0.0f, false, 4},   // 180 guard
    {50, 90.0f, false, 4},  // 181
    {50, 180.0f, false, 4}, // 182
    {50, 270.0f, false, 4}, // 183
    {50, 0.0f, true, 4},    // 184
    {50, 90.0f, true, 4},   // 185
    {50, 180.0f, true, 4},  // 186
    {50, 270.0f, true, 4},  // 187

    {238, 0.0f, false, 4},   // 188 officer
    {238, 90.0f, false, 4},  // 189
    {238, 180.0f, false, 4}, // 190
    {238, 270.0f, false, 4}, // 191
    {238, 0.0f, true, 4},    // 192
    {238, 90.0f, true, 4},   // 193
    {238, 180.0f, true, 4},  // 194
    {238, 270.0f, true, 4},  // 195

    {238, 270.0f, true, 1}, // 196 boss
    {238, 270.0f, true, 1}, // 197 boss

    {138, 0.0f, false, 4},   // 198 blue
    {138, 90.0f, false, 4},  // 199
    {138, 180.0f, false, 4}, // 200
    {138, 270.0f, false, 4}, // 201
    {138, 0.0f, true, 4},    // 202
    {138, 90.0f, true, 4},   // 203
    {138, 180.0f, true, 4},  // 204
    {138, 270.0f, true, 4},  // 205

    {138, 270.0f, true, 1}, // 206 none
    {138, 270.0f, true, 1}, // 207
    {138, 270.0f, true, 1}, // 208
    {138, 270.0f, true, 1}, // 209

    {99, 0.0f, true, 4},   // 210 dog
    {99, 90.0f, true, 4},  // 211
    {99, 180.0f, true, 4}, // 212
    {99, 270.0f, true, 4}, // 213

    {99, 270.0f, true, 1}, // 214 boss
    {99, 270.0f, true, 1}, // 215 boss

    {187, 0.0f, false, 1},   // 216 zombie
    {187, 90.0f, false, 1},  // 217
    {187, 180.0f, false, 1}, // 218
    {187, 270.0f, false, 1}, // 219
    {187, 0.0f, true, 1},    // 220
    {187, 90.0f, true, 1},   // 221
    {187, 180.0f, true, 1},  // 222
    {187, 270.0f, true, 1},  // 223

    {288, 270.0f, true, 1}, // 224 ghost
    {290, 270.0f, true, 1}, // 225
    {292, 270.0f, true, 1}, // 226
    {294, 270.0f, true, 1}, // 227

    {138, 270.0f, true, 1}, // 228 none
    {138, 270.0f, true, 1}, // 229
    {138, 270.0f, true, 1}, // 230
    {138, 270.0f, true, 1}, // 231
    {138, 270.0f, true, 1}, // 232
    {138, 270.0f, true, 1}, // 233

    {187, 0.0f, false, 3},   // 234 zombie
    {187, 90.0f, false, 3},  // 235
    {187, 180.0f, false, 3}, // 236
    {187, 270.0f, false, 3}, // 237
    {187, 0.0f, true, 3},    // 238
    {187, 90.0f, true, 3},   // 239
    {187, 180.0f, true, 3},  // 240
    {187, 270.0f, true, 3},  // 241

    {138, 270.0f, true, 1}, // 242 none
    {138, 270.0f, true, 1}, // 243
    {138, 270.0f, true, 1}, // 244
    {138, 270.0f, true, 1}, // 245
    {138, 270.0f, true, 1}, // 246
    {138, 270.0f, true, 1}, // 247
    {138, 270.0f, true, 1}, // 248
    {138, 270.0f, true, 1}, // 249
    {138, 270.0f, true, 1}, // 250
    {138, 270.0f, true, 1}, // 251

    {187, 0.0f, false, 4},   // 252 zombie
    {187, 90.0f, false, 4},  // 253
    {187, 180.0f, false, 4}, // 254
    {187, 270.0f, false, 4}, // 255
    {187, 0.0f, true, 4},    // 256
    {187, 90.0f, true, 4},   // 257
    {187, 180.0f, true, 4},  // 258
    {187, 270.0f, true, 4},  // 259
};

class Loaders
{
  public:
    Loaders(const std::filesystem::path& dataPath);

    Bitmap LoadPictureTexture(int pictureIndex);
    Bitmap LoadWallTextures();
    Bitmap LoadSpriteTextures();
    std::shared_ptr<Map> LoadMap(int episode, int level);

  private:
    std::filesystem::path _dataPath;
};
} // namespace Wolf3dLoaders