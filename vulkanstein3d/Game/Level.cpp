#include "../Common.h"

#include "Components.h"
#include "Intersection.h"
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
        if (map->tiles[0][i] > 53)
            continue;

        // Secret doors
        if (map->tiles[1][i] == 98)
            continue;

        _tileMap[i] = TileBlocksMovement | TileBlocksShooting;
    }

    for (int i = 0; i < map->tiles[0].size(); i++)
    {
        if (map->tiles[0][i] == 90)
            CreateDoorEntity(i, DoorVertical);
        if (map->tiles[0][i] == 91)
            CreateDoorEntity(i, 0);
        if (map->tiles[0][i] == 92)
            CreateDoorEntity(i, DoorVertical | DoorGoldKey);
        if (map->tiles[0][i] == 93)
            CreateDoorEntity(i, DoorGoldKey);
        if (map->tiles[0][i] == 94)
            CreateDoorEntity(i, DoorVertical | DoorSilverKey);
        if (map->tiles[0][i] == 95)
            CreateDoorEntity(i, DoorSilverKey);
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

        CreateSceneryEntity(i, objectId);
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
    _registry.emplace<Trigger>(entity, 5.0f);
}

void Level::CreateSceneryEntity(int index, int objectId)
{
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

void Level::CreateDoorEntity(int index, uint32_t flags)
{
    const auto entity = _registry.create();

    auto scale = (flags & Game::DoorVertical) ? glm::vec3{2.5f, 10.0f, 10.0f} : glm::vec3{10.0f, 10.0f, 2.5f};

    _registry.emplace<Transform>(entity, IndexToPosition(index, 5.0f), scale);
    _registry.emplace<Door>(entity, flags);
    _registry.emplace<Renderable>(entity, 0);
}

void Level::Update(double delta)
{
    auto& playerTrans = _registry.get<Game::Transform>(_player);
    auto& playerComponent = _registry.get<Game::Player>(_player);

    std::vector<entt::entity> remove;

    // Pick up items
    auto view = _registry.view<Game::Item, Game::Trigger, Game::Transform>();
    for (auto [entity, item, trigger, transform] : view.each())
    {
        if (Game::Intersection::CircleCircleIntersect({transform.position.x, transform.position.z}, trigger.radius, {playerTrans.position.x, playerTrans.position.z}, 5.0f))
        {
            switch ((Wolf3dLoaders::MapObjects)item.type)
            {
            case Wolf3dLoaders::MapObjects::GoldKey:
                playerComponent.hasGoldKey = true;
                break;
            case Wolf3dLoaders::MapObjects::SilverKey:
                playerComponent.hasSilverKey = true;
                break;
            case Wolf3dLoaders::MapObjects::Food:
                playerComponent.health += 10;
                break;
            case Wolf3dLoaders::MapObjects::FirstAidKit:
                playerComponent.health += 25;
                break;
            case Wolf3dLoaders::MapObjects::Ammo:
                playerComponent.ammo += 8;
                break;
            case Wolf3dLoaders::MapObjects::MachineGun:
                playerComponent.hasMachineGun = true;
                break;
            case Wolf3dLoaders::MapObjects::Gatling:
                playerComponent.hasGatling = true;
                break;
            case Wolf3dLoaders::MapObjects::Cross:
                playerComponent.score += 100;
                break;
            case Wolf3dLoaders::MapObjects::Chalice:
                playerComponent.score += 500;
                break;
            case Wolf3dLoaders::MapObjects::Jewels:
                playerComponent.score += 1000;
                break;
            case Wolf3dLoaders::MapObjects::Crown:
                playerComponent.score += 5000;
                break;
            case Wolf3dLoaders::MapObjects::ExtraLife:
                playerComponent.health += 100;
                playerComponent.ammo += 25;
                break;
            }

            if (playerComponent.ammo > 99)
                playerComponent.ammo = 99;
            if (playerComponent.health > 100)
                playerComponent.health = 100;

            spdlog::info("Picked up {}. Ammo:{} Health:{} Score:{}", item.type, playerComponent.ammo, playerComponent.health, playerComponent.score);
            remove.push_back(entity);
        }
    }

    _registry.destroy(remove.begin(), remove.end());
}

} // namespace Game