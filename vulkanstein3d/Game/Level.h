#pragma once

#include "../Rendering/Renderer.h"
#include "Components.h"
#include "MeshGenerator.h"

#include "entt/entt.hpp"

namespace Rendering
{
class Device;
}

namespace Wolf3dLoaders
{
struct Map;
}

namespace Game
{

constexpr uint32_t TileBlocksMovement = 0x1;
constexpr uint32_t TileBlocksShooting = 0x2;

class Level
{
  public:
    Level(Rendering::Renderer& renderer, std::shared_ptr<Wolf3dLoaders::Map> map);

    std::shared_ptr<Wolf3dLoaders::Map> GetMap() { return _map; }
    const std::vector<uint32_t>& GetTiles() { return _tileMap; }

    entt::registry& GetRegistry() { return _registry; }
    entt::entity GetPlayerEntity() { return _player; }

    void Update(double delta);

    Rendering::Mesh _mapMesh;
    Rendering::Mesh _floorMesh;

  private:
    void CreateEntities();
    void CreatePlayerEntity(int index);
    void CreateItemEntity(int index);
    void CreateSceneryEntity(int index, int objectId);
    void CreateDoorEntity(int index, uint32_t flags);
    void CreateSecretDoorEntity(int index);

    glm::vec3 IndexToPosition(int index, float height = 0.0f);

    void UpdateInput(double delta);
    void UpdateDoors(double delta);

    bool IsCollision(const glm::vec3& pos);

    void Activate();
    void ActivateDoor(entt::entity doorEntity);

  private:
    Rendering::Renderer& _renderer;
    std::shared_ptr<Wolf3dLoaders::Map> _map;
    std::vector<uint32_t> _tileMap;

    entt::registry _registry;
    entt::entity _player{entt::null};
};
} // namespace Game