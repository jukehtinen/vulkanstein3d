#pragma once

#include "../Rendering/Mesh.h"
#include "../Wolf3dLoaders/Loaders.h"

namespace Rendering
{
class Buffer;
class Device;
} // namespace Rendering

namespace Game
{
class MeshGenerator
{
  public:
    static Rendering::Mesh BuildFloorPlaneMesh(std::shared_ptr<Rendering::Device> device, int size);
    static Rendering::Mesh BuildCubeMesh(std::shared_ptr<Rendering::Device> device);
    static Rendering::Mesh BuildMapMesh(std::shared_ptr<Rendering::Device> device, const Wolf3dLoaders::Map& map);
};
} // namespace Game