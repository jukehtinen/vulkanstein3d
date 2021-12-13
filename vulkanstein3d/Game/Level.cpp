#include "../Common.h"

#include "Components.h"
#include "Level.h"

#include "../Wolf3dLoaders/Loaders.h"

namespace Game
{
Level::Level(std::shared_ptr<Wolf3dLoaders::Map> map)
    : _map(map)
{
    using namespace Wolf3dLoaders;

    _tileMap.resize(map->width * map->width);
    for (int i = 0; i < map->tiles[0].size(); i++)
    {
        if (map->tiles[0][i] > 107)
            continue;

        _tileMap[i] = TileBlocksMovement | TileBlocksShooting;
    }

    for (int i = 0; i < map->tiles[1].size(); i++)
    {
        int objectId = map->tiles[1][i];
        if (objectId == 0)
            continue;

        // Enemies and such
        if (objectId >= 90)
            continue;

        switch ((Wolf3dLoaders::MapObjects)objectId)
        {
        case MapObjects::PlayerNorth:
        case MapObjects::PlayerEast:
        case MapObjects::PlayerSouth:
        case MapObjects::PlayerWest:
            CreatePlayerEntity(i);
            continue;
        case MapObjects::GoldKey:
        case MapObjects::SilverKey:
        case MapObjects::Food:
        case MapObjects::FirstAidKit:
        case MapObjects::Ammo:
        case MapObjects::MachineGun:
        case MapObjects::Gatling:
        case MapObjects::Cross:
        case MapObjects::Chalice:
        case MapObjects::Jewels:
        case MapObjects::Crown:
        case MapObjects::ExtraLife:
            CreateItemEntity(i);
            continue;
        }

        CreateSceneryEntity(i);
    }
}

glm::vec3 Level::IndexToPosition(int index, float height)
{
    return glm::vec3{index % _map->width * 10.0f + 5.0f, height, index / _map->width * 10.0f + 5.0f};
}

void Level::CreatePlayerEntity(int index)
{
    assert(_player == entt::null);

    _player = _registry.create();
    _registry.emplace<Player>(_player);
    _registry.emplace<Transform>(_player, IndexToPosition(index, 35.0f));
}

void Level::CreateItemEntity(int index)
{
    const auto entity = _registry.create();
    _registry.emplace<Item>(entity, _map->tiles[1][index]);
    _registry.emplace<Sprite>(entity, _map->tiles[1][index] - 21);
    _registry.emplace<Transform>(entity, IndexToPosition(index, 5.0f));
    _registry.emplace<Trigger>(entity, 3.5f);
}

void Level::CreateSceneryEntity(int index)
{
    int objectId = _map->tiles[1][index];
    const auto entity = _registry.create();
    _registry.emplace<Sprite>(entity, objectId - 21);
    _registry.emplace<Transform>(entity, IndexToPosition(index, 5.0f));

    const std::vector<int> blocksShooting{30, 38, 39, 67, 69};
    const std::vector<int> blocksMovement{24, 25, 26, 28, 30, 31, 32, 33, 34, 35, 36, 38, 39, 40, 41, 58, 59, 60, 62, 63, 67, 68, 69};

    uint32_t flags = 0;
    if (std::find(blocksShooting.begin(), blocksShooting.end(), objectId) != blocksShooting.end())
        flags |= Game::TileBlocksShooting;
    if (std::find(blocksMovement.begin(), blocksMovement.end(), objectId) != blocksMovement.end())
        flags |= Game::TileBlocksMovement;

    _tileMap[index] = flags;
}

} // namespace Game