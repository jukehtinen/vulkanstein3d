#pragma once

namespace Game
{
struct Item
{
    int type{0};
};

struct Player
{
};

struct Sprite
{
    int spriteIndex{0};
};

struct Transform
{
    glm::vec3 position;
};

struct Trigger
{
    float radius{0};
};

} // namespace Game