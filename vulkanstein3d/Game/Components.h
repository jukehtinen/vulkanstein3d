#pragma once

namespace Game
{
constexpr uint32_t DoorVertical = 0x1;
constexpr uint32_t DoorSilverKey = 0x2;
constexpr uint32_t DoorGoldKey = 0x4;
constexpr uint32_t DoorElevator = 0x8;

struct Door
{
    uint32_t flags{0};
};

struct Item
{
    int type{0};
};

struct Renderable
{
    int type{0};
};

struct Player
{
    bool hasGoldKey{false};
    bool hasSilverKey{false};
    bool hasMachineGun{false};
    bool hasGatling{false};
    int health{100};
    int score{0};
    int ammo{0};
};

struct Sprite
{
    int spriteIndex{0};
};

struct Transform
{
    glm::vec3 position;
    glm::vec3 scale{1.0f};
};

struct Trigger
{
    float radius{0};
};

} // namespace Game