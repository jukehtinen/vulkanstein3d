#pragma once

namespace Game
{
constexpr uint32_t DoorVertical = 0x1;
constexpr uint32_t DoorSilverKey = 0x2;
constexpr uint32_t DoorGoldKey = 0x4;
constexpr uint32_t DoorElevator = 0x8;

struct Collider
{
    int type{0};
};

struct Door
{
    enum class State
    {
        Closed,
        Opening,
        Open,
        Closing
    };
    uint32_t flags{0};
    Door::State state{State::Closed};
    float time{0.0f};
    glm::vec3 doorClosedPos{0.0f};
    glm::vec3 doorOpenPos{0.0f};
};

struct Elevator
{
    enum class Type
    {
        Normal,
        SecretLevel
    };
    Elevator::Type type{Type::Normal};
    bool isActivated{false};
};

struct Item
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

struct Renderable
{
    int tileIndex{0};
};

struct SecretDoor
{
    enum class State
    {
        Closed,
        Opening,
        Open
    };
    SecretDoor::State state{State::Closed};
    float time{0.0f};
    glm::vec3 doorClosedPos{0.0f};
    glm::vec3 doorOpenPos{0.0f};
};

struct FPSCamera
{
    glm::vec3 front{0.0f, 0.0f, -1.0f};
    glm::vec3 up{0.0f, 1.0f, 0.0f};
    glm::vec3 right;
    glm::vec3 worldUp{0.0f, 1.0f, 0.0f};

    float yaw = 0.0f;
    float pitch = 0.0f;

    bool firstMouse{true};
    float lastX{0.0f};
    float lastY{0.0f};
};

struct Sprite
{
    int spriteIndex{0};
};

struct SpriteAnimation
{
    int baseIndex{0};
    float facingAngle{0};
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