#pragma once

#include "Components.h"
#include "entt/entt.hpp"

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
    Level(std::shared_ptr<Wolf3dLoaders::Map> map);

    std::shared_ptr<Wolf3dLoaders::Map> GetMap() { return _map; }
    const std::vector<uint32_t>& GetTiles() { return _tileMap; }

    entt::registry& GetRegistry() { return _registry; }
    entt::entity GetPlayerEntity() { return _player; }

    void Update(double delta);

  private:
    void CreatePlayerEntity(int index);
    void CreateItemEntity(int index);
    void CreateSceneryEntity(int index, int objectId);
    void CreateDoorEntity(int index, uint32_t flags);

    glm::vec3 IndexToPosition(int index, float height = 0.0f);

  private:
    std::shared_ptr<Wolf3dLoaders::Map> _map;
    std::vector<uint32_t> _tileMap;

    entt::registry _registry;

    entt::entity _player{entt::null};
};
} // namespace Game