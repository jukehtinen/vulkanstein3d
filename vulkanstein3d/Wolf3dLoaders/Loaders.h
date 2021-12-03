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
    Bitmap LoadWallTexture(int index);
    Map LoadMap(int episode, int level);



  private:
    std::filesystem::path _dataPath;
};
} // namespace Wolf3dLoaders