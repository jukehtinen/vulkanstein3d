#pragma once

#include <filesystem>
#include <map>

namespace Rendering
{
class Device;
class Material;
class Texture;
} // namespace Rendering

namespace Game
{
class Assets
{
  public:
    Assets(std::shared_ptr<Rendering::Device> device, const std::filesystem::path& dataPath);
    ~Assets();

    std::shared_ptr<Rendering::Texture> GetTexture(const std::string& name);
    void AddTexture(const std::string& name, std::shared_ptr<Rendering::Texture> texture);

    std::shared_ptr<Rendering::Material> GetMaterial(const std::string& name);
    void AddMaterial(const std::string& name, std::shared_ptr<Rendering::Material> texture);

  private:
    std::map<std::string, std::shared_ptr<Rendering::Texture>> _textures;
    std::map<std::string, std::shared_ptr<Rendering::Material>> _materials;
};
} // namespace Game
