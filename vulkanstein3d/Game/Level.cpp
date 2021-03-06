#include "../Common.h"

#include "Components.h"
#include "Intersection.h"
#include "Level.h"
#include "MeshGenerator.h"

#include "../App/Input.h"
#include "../Rendering/Device.h"
#include "../Wolf3dLoaders/Loaders.h"

namespace Game
{
constexpr float DoorMoveTime = 0.75f;
constexpr float DoorStayOpenTime = 3.0f;
constexpr float SecretDoorMoveTime = 2.0f;
constexpr float WeaponChangeTime = 0.2f;

constexpr int TileElevatorSwitchOff = 41;
constexpr int TileElevatorSwitchOn = 43;
constexpr int TileElevatorToSecretFloor = 107;

glm::ivec2 GetTile(const glm::vec3& worldPos)
{
    return {(int)(worldPos.x / 10.0f), (int)(worldPos.z / 10.0f)};
}

Level::Level(Rendering::Renderer& renderer, std::shared_ptr<Wolf3dLoaders::Map> map)
    : _renderer(renderer), _map(map)
{
    _floorMesh = Game::MeshGenerator::BuildFloorPlaneMesh(renderer._device, map->width);
    _mapMesh = Game::MeshGenerator::BuildMapMesh(renderer._device, *map.get());

    _tileMap.resize(map->width * map->width);
    for (int i = 0; i < map->tiles[0].size(); i++)
    {
        if (map->tiles[0][i] > 53)
            continue;

        // Secret doors
        if (map->tiles[1][i] == 98)
            continue;

        // Elevator
        if (map->tiles[0][i] == 21 && (map->tiles[0][i - 1] >= 90 || map->tiles[0][i + 1] >= 90))
            continue;

        _tileMap[i] = TileBlocksMovement | TileBlocksShooting;
    }

    CreateEntities();

    auto& playerXform = _registry.get<Game::Transform>(GetPlayerEntity());
    playerXform.position.y = 5.5f;
}

glm::vec3 Level::IndexToPosition(int index, float height)
{
    return glm::vec3{index % _map->width * 10.0f + 5.0f, height, index / _map->width * 10.0f + 5.0f};
}

void Level::CreateEntities()
{
    using namespace Wolf3dLoaders;

    for (int i = 0; i < _map->tiles[0].size(); i++)
    {
        if (_map->tiles[0][i] == 21 && (_map->tiles[0][i - 1] >= 90 || _map->tiles[0][i + 1] >= 90))
            CreateElevatorEntity(i);
        if (_map->tiles[0][i] == 90)
            CreateDoorEntity(i, DoorVertical);
        if (_map->tiles[0][i] == 91)
            CreateDoorEntity(i, 0);
        if (_map->tiles[0][i] == 92)
            CreateDoorEntity(i, DoorVertical | DoorGoldKey);
        if (_map->tiles[0][i] == 93)
            CreateDoorEntity(i, DoorGoldKey);
        if (_map->tiles[0][i] == 94)
            CreateDoorEntity(i, DoorVertical | DoorSilverKey);
        if (_map->tiles[0][i] == 95)
            CreateDoorEntity(i, DoorSilverKey);
        if (_map->tiles[0][i] == 100)
            CreateDoorEntity(i, DoorVertical | DoorElevator);
    }

    for (int i = 0; i < _map->tiles[1].size(); i++)
    {
        int objectId = _map->tiles[1][i];
        if (objectId == 0)
            continue;

        switch ((Wolf3dLoaders::MapObjects)objectId)
        {
        case MapObjects::PlayerNorth:
        case MapObjects::PlayerEast:
        case MapObjects::PlayerSouth:
        case MapObjects::PlayerWest:
            CreatePlayerEntity(i, objectId);
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
        case MapObjects::SecretDoor:
            CreateSecretDoorEntity(i);
            continue;
        case MapObjects::EndGameTrigger:
            // todo
            continue;
        }

        if (objectId < 90)
        {
            CreateSceneryEntity(i, objectId);
            continue;
        }

        if (objectId >= (int)MapObjects::EnemyFirst)
        {
            CreateEnemyEntity(i, objectId);
            continue;
        }
    }
}

void Level::CreatePlayerEntity(int index, int objectId)
{
    assert(_player == entt::null);

    _player = _registry.create();
    _registry.emplace<Player>(_player);
    _registry.emplace<Transform>(_player, IndexToPosition(index, 35.0f));
    auto& fpsCamera = _registry.emplace<FPSCamera>(_player);
    if (objectId == (int)Wolf3dLoaders::MapObjects::PlayerNorth)
        fpsCamera.yaw = 270;
    if (objectId == (int)Wolf3dLoaders::MapObjects::PlayerEast)
        fpsCamera.yaw = 0;
    if (objectId == (int)Wolf3dLoaders::MapObjects::PlayerSouth)
        fpsCamera.yaw = 90;
    if (objectId == (int)Wolf3dLoaders::MapObjects::PlayerWest)
        fpsCamera.yaw = 180;
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
    const std::vector<int> blocksMovement{24, 25, 26, 28, 30, 31, 33, 34, 35, 36, 38, 39, 40, 41, 58, 59, 60, 62, 63, 67, 68, 69};

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

    int tileId = 98;
    if ((flags & Game::DoorElevator))
        tileId = 102;
    else if ((flags & Game::DoorGoldKey) || (flags & Game::DoorSilverKey))
        tileId = 104;

    _registry.emplace<Transform>(entity, IndexToPosition(index, 5.0f), scale);
    _registry.emplace<Door>(entity, flags);
    _registry.emplace<Collider>(entity);
    _registry.emplace<Renderable>(entity, tileId);
}

void Level::CreateSecretDoorEntity(int index)
{
    const auto entity = _registry.create();

    int tileId = _map->tiles[0][index] - 1;
    tileId *= 2;
    if (tileId % 2 != 0)
        tileId++;
    _registry.emplace<Transform>(entity, IndexToPosition(index, 5.0f), glm::vec3{10.0f});
    _registry.emplace<SecretDoor>(entity);
    _registry.emplace<Collider>(entity);
    _registry.emplace<Renderable>(entity, tileId);
}

void Level::CreateElevatorEntity(int index)
{
    const auto entity = _registry.create();

    bool isSecret = _map->tiles[0][index - 1] == TileElevatorToSecretFloor || _map->tiles[0][index + 1] == TileElevatorToSecretFloor;

    _registry.emplace<Transform>(entity, IndexToPosition(index, 5.0f), glm::vec3{10.0f});
    _registry.emplace<Elevator>(entity, isSecret ? Elevator::Type::SecretLevel : Elevator::Type::Normal);
    _registry.emplace<Collider>(entity);
    _registry.emplace<Renderable>(entity, TileElevatorSwitchOff);
}

void Level::CreateEnemyEntity(int index, int objectId)
{
    if (objectId == 124) // dead guard
        return;

    auto& mob = Wolf3dLoaders::Enemies[objectId - (int)Wolf3dLoaders::MapObjects::EnemyFirst];

    const auto entity = _registry.create();
    _registry.emplace<Transform>(entity, IndexToPosition(index, 5.0f), glm::vec3{10.0f});
    _registry.emplace<Sprite>(entity, 50);
    _registry.emplace<SpriteAnimation>(entity, mob.baseIndex, mob.angle);
    //_registry.emplace<Collider>(entity);
}

void Level::Update(double delta)
{
    auto& playerXform = _registry.get<Game::Transform>(_player);
    auto& playerComponent = _registry.get<Game::Player>(_player);

    UpdateInput(delta);
    UpdateDoors(delta);
    UpdateWeapon(delta);
    UpdateAnimations(delta);

    std::vector<entt::entity> remove;

    // Pick up items
    auto view = _registry.view<Game::Item, Game::Trigger, Game::Transform>();
    for (auto [entity, item, trigger, transform] : view.each())
    {
        if (Game::Intersection::CircleCircleIntersect({transform.position.x, transform.position.z}, trigger.radius, {playerXform.position.x, playerXform.position.z}, 5.0f))
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

void Level::UpdateInput(double delta)
{
    const float MaxSpeedRun = 55.0f;
    const float MaxSpeedWalk = 35.0f;
    const float Sensitivity = 0.1f;

    auto& playerXform = _registry.get<Game::Transform>(_player);
    auto& fpsCamera = _registry.get<Game::FPSCamera>(_player);

    glm::vec3 prevPos = playerXform.position;
    glm::vec3 newPos = playerXform.position;

    const float ypos = prevPos.y;

    auto& input = App::Input::The();

    float currentSpeed = MaxSpeedRun;
    if (input.IsKeyDown(GLFW_KEY_LEFT_SHIFT) || input.IsKeyDown(GLFW_KEY_RIGHT_SHIFT))
        currentSpeed = MaxSpeedWalk;

    float velocity = currentSpeed * (float)delta;
    if (input.IsKeyDown(GLFW_KEY_W))
        newPos += fpsCamera.front * velocity;
    if (input.IsKeyDown(GLFW_KEY_S))
        newPos -= fpsCamera.front * velocity;
    if (input.IsKeyDown(GLFW_KEY_A))
        newPos -= fpsCamera.right * velocity;
    if (input.IsKeyDown(GLFW_KEY_D))
        newPos += fpsCamera.right * velocity;
    if (input.IsButtonTapped(0))
        Activate();
    if (input.IsButtonTapped(1))
        _state = LevelState::GoToNextLevel;

    auto mousepos = input.GetMousePos();
    if (fpsCamera.firstMouse && mousepos.x != 0 && mousepos.y != 0)
    {
        fpsCamera.lastX = (float)mousepos.x;
        fpsCamera.lastY = (float)mousepos.y;
        fpsCamera.firstMouse = false;
    }

    float xoffset = (mousepos.x - fpsCamera.lastX) * Sensitivity;
    float yoffset = (fpsCamera.lastY - mousepos.y) * Sensitivity;

    fpsCamera.lastX = (float)mousepos.x;
    fpsCamera.lastY = (float)mousepos.y;

    fpsCamera.yaw += xoffset;
    fpsCamera.pitch += yoffset;

    if (fpsCamera.pitch > 89.0f)
        fpsCamera.pitch = 89.0f;
    if (fpsCamera.pitch < -89.0f)
        fpsCamera.pitch = -89.0f;

    newPos.y = ypos;

    glm::vec3 front{glm::cos(glm::radians(fpsCamera.yaw)) * glm::cos(glm::radians(fpsCamera.pitch)), glm::sin(glm::radians(fpsCamera.pitch)), glm::sin(glm::radians(fpsCamera.yaw)) * glm::cos(glm::radians(fpsCamera.pitch))};
    fpsCamera.front = glm::normalize(front);
    fpsCamera.right = glm::normalize(glm::cross(fpsCamera.front, fpsCamera.worldUp));
    fpsCamera.up = glm::normalize(glm::cross(fpsCamera.right, fpsCamera.front));

    playerXform.position.x = newPos.x;
    if (IsCollision(playerXform.position))
        playerXform.position.x = prevPos.x;

    playerXform.position.z = newPos.z;
    if (IsCollision(playerXform.position))
        playerXform.position.z = prevPos.z;
}

bool Level::IsCollision(const glm::vec3& pos)
{
    const auto tile = GetTile(pos);

    const auto& tiles = GetTiles();
    for (int y = tile.y - 1; y < tile.y + 2; y++)
    {
        for (int x = tile.x - 1; x < tile.x + 2; x++)
        {
            glm::vec4 rect{x * 10.0f + 5.0f, y * 10.0f + 5.0f, 10.0f, 10.0f};
            if ((tiles[y * 64 + x] & Game::TileBlocksMovement) && Game::Intersection::CircleRectIntersect({pos.x, pos.z}, 3.0f, rect))
                return true;
        }
    }

    auto colliderView = _registry.view<Game::Transform, Game::Collider>();
    for (auto [entity, xform, collider] : colliderView.each())
    {
        const auto colliderTile = GetTile(xform.position);

        glm::vec4 rect{colliderTile.x * 10.0f + 5.0f, colliderTile.y * 10.0f + 5.0f, 10.0f, 10.0f};
        if (Game::Intersection::CircleRectIntersect({pos.x, pos.z}, 3.0f, rect))
            return true;
    }

    return false;
}

void Level::Activate()
{
    const auto& playerXform = _registry.get<Game::Transform>(GetPlayerEntity());
    const auto& fpsCamera = _registry.get<Game::FPSCamera>(GetPlayerEntity());
    const auto& player = _registry.get<Game::Player>(GetPlayerEntity());

    const auto activatePosition = playerXform.position + (fpsCamera.front * 7.5f);
    const auto activateTile = GetTile(activatePosition);

    auto colliderView = _registry.view<Game::Transform, Game::Collider>();
    for (auto [entity, xform, collider] : colliderView.each())
    {
        const auto colliderTile = GetTile(xform.position);
        if (activateTile == colliderTile)
        {
            auto doorComponent = _registry.try_get<Game::Door>(entity);
            if (doorComponent != nullptr)
            {
                if (doorComponent->state == Door::State::Closed)
                {
                    if (doorComponent->flags & Game::DoorGoldKey && !player.hasGoldKey)
                    {
                        spdlog::info("You need the gold key.");
                        return;
                    }
                    if (doorComponent->flags & Game::DoorSilverKey && !player.hasSilverKey)
                    {
                        spdlog::info("You need the silver key.");
                        return;
                    }

                    auto vertical = (doorComponent->flags & Game::DoorVertical);
                    doorComponent->state = Door::State::Opening;
                    doorComponent->time = DoorMoveTime;
                    doorComponent->doorClosedPos = xform.position;
                    doorComponent->doorOpenPos = xform.position + (vertical ? glm::vec3{0.0f, 0.0f, 12.0f} : glm::vec3{12.0f, 0.0f, 0.0f});
                    return;
                }
            }
            auto secretDoorComponent = _registry.try_get<Game::SecretDoor>(entity);
            if (secretDoorComponent != nullptr)
            {
                if (secretDoorComponent->state == SecretDoor::State::Closed)
                {
                    const auto playerTile = GetTile(playerXform.position);

                    glm::vec3 openOffset;
                    if (playerTile.x == colliderTile.x)
                        openOffset = {0.0f, 0.0f, playerTile.y > colliderTile.y ? -20.0f : 20.0f};
                    else if (playerTile.y == colliderTile.y)
                        openOffset = {playerTile.x > colliderTile.x ? -20.0f : 20.0f, 0.0f, 0.0f};
                    else
                        return;

                    secretDoorComponent->state = SecretDoor::State::Opening;
                    secretDoorComponent->time = SecretDoorMoveTime;
                    secretDoorComponent->doorClosedPos = xform.position;
                    secretDoorComponent->doorOpenPos = xform.position + openOffset;
                    return;
                }
            }
            auto elevatorComponent = _registry.try_get<Game::Elevator>(entity);
            if (elevatorComponent != nullptr)
            {
                auto rendeableComponent = _registry.try_get<Game::Renderable>(entity);
                elevatorComponent->isActivated = !elevatorComponent->isActivated;
                rendeableComponent->tileIndex = elevatorComponent->isActivated ? TileElevatorSwitchOn : TileElevatorSwitchOff;

                _state = elevatorComponent->type == Elevator::Type::Normal ? LevelState::GoToNextLevel : LevelState::GoToSecretLevel;
            }
        }
    }
}

void Level::UpdateDoors(double delta)
{
    auto doorView = _registry.view<Game::Transform, Game::Door>();
    for (auto [entity, xform, door] : doorView.each())
    {
        door.time -= (float)delta;

        switch (door.state)
        {
        case Door::State::Opening: {
            xform.position = glm::mix(door.doorClosedPos, door.doorOpenPos, (DoorMoveTime - door.time) / DoorMoveTime);
            if (door.time <= 0.0f)
            {
                door.state = Door::State::Open;
                door.time = DoorStayOpenTime;
            }
            break;
        }
        case Door::State::Open: {
            if (door.time <= 0.0f)
            {
                // Don't close if player is still standing nearby.
                const auto& playerXform = _registry.get<Game::Transform>(GetPlayerEntity());
                if (glm::distance(playerXform.position, door.doorClosedPos) < 10.0f)
                {
                    door.time = DoorStayOpenTime;
                    break;
                }

                door.state = Door::State::Closing;
                door.time = DoorMoveTime;
            }
            break;
        }
        case Door::State::Closing: {
            xform.position = glm::mix(door.doorOpenPos, door.doorClosedPos, (DoorMoveTime - door.time) / DoorMoveTime);
            if (door.time <= 0.0f)
            {
                door.state = Door::State::Closed;
            }
            break;
        }
        default:
            break;
        }
    }

    auto secretDoorView = _registry.view<Game::Transform, Game::SecretDoor>();
    for (auto [entity, xform, door] : secretDoorView.each())
    {
        door.time -= (float)delta;

        switch (door.state)
        {
        case SecretDoor::State::Opening: {
            xform.position = glm::mix(door.doorClosedPos, door.doorOpenPos, (SecretDoorMoveTime - door.time) / SecretDoorMoveTime);
            if (door.time <= 0.0f)
                door.state = SecretDoor::State::Open;
            break;
        }
        default:
            break;
        }
    }
}

void Level::UpdateWeapon(double delta)
{
    auto& input = App::Input::The();

    auto& player = _registry.get<Game::Player>(GetPlayerEntity());

    auto changeWeapon = [&](Weapon w) {
        if (_weaponState != WeaponState::Ready)
            return;

        _weaponChangeTimer = 0.0f;
        _nextWeapon = w;
        _weaponState = WeaponState::ChangingDown;
    };

    if (input.IsKeyDown(GLFW_KEY_1))
        changeWeapon(Weapon::Knife);
    if (input.IsKeyDown(GLFW_KEY_2))
        changeWeapon(Weapon::Pistol);
    if (input.IsKeyDown(GLFW_KEY_3))
        changeWeapon(Weapon::MachineGun);
    if (input.IsKeyDown(GLFW_KEY_4))
        changeWeapon(Weapon::Gatling);

    // fixme: complicated state hacking, maybe have own func for each weapon.
    if (input.IsButtonDown(2))
    {
        if (_weaponState == WeaponState::Ready)
        {
            if (_currentWeapon == Weapon::Knife || player.ammo > 0)
            {
                _weaponState = WeaponState::Firing;
                _weaponFrameOffset = 0;
                _weaponChangeTimer = 0.0f;
            }
        }
        if (player.ammo == 0)
            changeWeapon(Weapon::Knife);
    }
    else
    {
        if (_weaponState == WeaponState::SemiAuto)
            _weaponState = WeaponState::Ready;
    }

    if (_weaponState == WeaponState::Firing)
    {
        _weaponChangeTimer += (float)delta;
        if (_weaponChangeTimer >= 0.075f)
        {
            _weaponFrameOffset++;
            _weaponChangeTimer = 0.0f;

            if (_weaponFrameOffset == 3 || (_currentWeapon == Weapon::Gatling && _weaponFrameOffset == 4))
            {
                if (_currentWeapon != Weapon::Knife)
                    player.ammo--;

                spdlog::info("Ammo: {}", player.ammo);
            }
        }

        if (_currentWeapon == Weapon::Knife || _currentWeapon == Weapon::Pistol)
        {
            if (_weaponFrameOffset >= 5)
            {
                _weaponState = WeaponState::SemiAuto;
                _weaponFrameOffset = 0;
            }
        }
        else
        {
            if (input.IsButtonDown(2) && player.ammo > 0)
            {
                if (_weaponFrameOffset >= 4)
                    _weaponFrameOffset = 2;
            }
            else
            {
                if (_weaponFrameOffset >= 5)
                {
                    _weaponState = WeaponState::Ready;
                    _weaponFrameOffset = 0;
                }
            }
        }
    }

    if (_weaponState == WeaponState::ChangingDown)
    {
        _weaponChangeTimer += (float)delta;
        _weaponChangeOffset = glm::clamp(1.0f - glm::pow(1.0f - (_weaponChangeTimer / WeaponChangeTime), 5.0f), 0.0f, 1.0f);
        if (_weaponChangeTimer >= WeaponChangeTime)
        {
            _weaponChangeTimer = 0.0f;
            _weaponState = WeaponState::ChangingUp;
            _currentWeapon = _nextWeapon;
        }
    }
    else if (_weaponState == WeaponState::ChangingUp)
    {
        _weaponChangeTimer += (float)delta;
        _weaponChangeOffset = 1.0f - glm::clamp(1.0f - glm::pow(1.0f - (_weaponChangeTimer / WeaponChangeTime), 5.0f), 0.0f, 1.0f);
        if (_weaponChangeTimer >= WeaponChangeTime)
        {
            _weaponChangeTimer = 0.0f;
            _weaponState = WeaponState::Ready;
        }
    }
}

void Level::UpdateAnimations(double delta)
{
    auto& playerXform = _registry.get<Game::Transform>(GetPlayerEntity());

    _animationTimer += (float)delta;
    auto animationView = _registry.view<Game::Transform, Game::Sprite, Game::SpriteAnimation>();
    for (auto [entity, xform, sprite, spriteAnim] : animationView.each())
    {
        if (_animationTimer > 0.25f)
        {
            //spriteAnim.baseIndex++;
        }

        const auto playerToMob = glm::normalize(playerXform.position - xform.position);

        auto angle = (int32_t)(glm::degrees(glm::atan(playerToMob.z, playerToMob.x)) + spriteAnim.facingAngle - (45 / 2));
        if (angle < 0)
            angle += 360;
        if (angle > 360)
            angle -= 360;
        angle = angle % 360;

        int frameRotationOffset = (7 - (int)(angle / 45));

        sprite.spriteIndex = spriteAnim.baseIndex + frameRotationOffset;
    }
    if (_animationTimer > 0.25f)
        _animationTimer = 0.0f;
}
} // namespace Game