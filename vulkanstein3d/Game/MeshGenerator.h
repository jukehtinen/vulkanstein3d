#pragma once

#include "../Wolf3dLoaders/Loaders.h"

namespace Rendering
{
class Buffer;
class Device;
} // namespace Rendering

namespace Game
{

class Mesh
{
public:
    std::shared_ptr<Rendering::Buffer> vertexBuffer;
    std::shared_ptr<Rendering::Buffer> indexBuffer;
    uint32_t indexCount{};
};

class MeshGenerator
{
  public:
    static Game::Mesh BuildFloorPlaneMesh(std::shared_ptr<Rendering::Device> device, int size);
    static Game::Mesh BuildCubeMesh(std::shared_ptr<Rendering::Device> device);
    static Game::Mesh BuildMapMesh(std::shared_ptr<Rendering::Device> device, const Wolf3dLoaders::Map& map);
};
} // namespace Game