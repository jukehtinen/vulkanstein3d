#pragma once

#include <array>
#include <filesystem>
#include <vector>

namespace Wolf3dLoaders
{
enum class Pictures
{
    Loading01 = 24,
    Loading02 = 25,
    Num0 = 45,
    LetterA = 56,
    LetterZ = 81,
    IntroScreen = 87,
    Knife = 91,
    Pistol = 92,
    Machinegun = 93,
    Gatling = 94,
    KeyFrame = 95,
    KeyGold = 96,
    KeySilver = 97,
    Num0White = 99,
    Last = 100
};

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
    ExtraLife = 56
};

struct Map
{
    std::array<std::vector<uint16_t>, 2> tiles;
    int width{};
};

class Loaders
{
  public:
    Loaders(const std::filesystem::path& dataPath);

    Bitmap LoadPictureTexture(Pictures picture);
    Bitmap LoadWallTextures();
    Bitmap LoadSpriteTextures();
    std::shared_ptr<Map> LoadMap(int episode, int level);

  private:
    std::filesystem::path _dataPath;
};
} // namespace Wolf3dLoaders