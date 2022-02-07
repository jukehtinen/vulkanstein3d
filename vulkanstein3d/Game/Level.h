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
    enum class LevelState
    {
        Playing,
        GoToNextLevel,
        GoToSecretLevel
    };

    enum class Weapon
    {
        Knife,
        Pistol,
        MachineGun,
        Gatling
    };

    enum class WeaponState
    {
        Ready,
        Firing,
        SemiAuto,
        ChangingDown,
        ChangingUp
    };

    Level(Rendering::Renderer& renderer, std::shared_ptr<Wolf3dLoaders::Map> map);

    std::shared_ptr<Wolf3dLoaders::Map> GetMap() { return _map; }
    const std::vector<uint32_t>& GetTiles() { return _tileMap; }

    entt::registry& GetRegistry() { return _registry; }
    entt::entity GetPlayerEntity() { return _player; }

    void Update(double delta);

    LevelState GetState() { return _state; }

    Rendering::Mesh _mapMesh;
    Rendering::Mesh _floorMesh;

    Weapon _currentWeapon{Weapon::Pistol};
    float _weaponChangeOffset{0.0f};
    int _weaponFrameOffset{0};

  private:
    void CreateEntities();
    void CreatePlayerEntity(int index, int objectId);
    void CreateItemEntity(int index);
    void CreateSceneryEntity(int index, int objectId);
    void CreateDoorEntity(int index, uint32_t flags);
    void CreateSecretDoorEntity(int index);
    void CreateElevatorEntity(int index);
    void CreateEnemyEntity(int index, int objectId);

    glm::vec3 IndexToPosition(int index, float height = 0.0f);

    void UpdateInput(double delta);
    void UpdateDoors(double delta);
    void UpdateWeapon(double delta);
    void UpdateAnimations(double delta);

    bool IsCollision(const glm::vec3& pos);

    void Activate();
    void ActivateDoor(entt::entity doorEntity);

  private:
    Rendering::Renderer& _renderer;
    std::shared_ptr<Wolf3dLoaders::Map> _map;
    std::vector<uint32_t> _tileMap;

    entt::registry _registry;
    entt::entity _player{entt::null};

    LevelState _state{LevelState::Playing};
    WeaponState _weaponState{WeaponState::Ready};
    Weapon _nextWeapon{Weapon::Pistol};
    float _weaponChangeTimer{0.0f};
    
    float _animationTimer{0.0f};
};
} // namespace Game