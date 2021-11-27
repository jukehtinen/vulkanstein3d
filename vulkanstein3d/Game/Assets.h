#pragma once

#include <filesystem>
#include <vector>

namespace Rendering
{
class Device;
class Texture;
} // namespace Rendering

namespace Game
{
class Assets
{
  public:
    Assets(std::shared_ptr<Rendering::Device> device, const std::filesystem::path& dataPath);
    ~Assets();

    std::vector<std::shared_ptr<Rendering::Texture>> _textures;
};
} // namespace Game
