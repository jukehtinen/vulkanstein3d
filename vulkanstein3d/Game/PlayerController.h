#pragma once

namespace Game
{
class PlayerController
{
  public:
    PlayerController(const glm::vec3& pos);

    void Update(float delta);

    const glm::mat4 GetViewMatrix();

    void SetPosition(const glm::vec3& pos) { _position = pos; }

  private:
    void UpdateVectors();

  private:
    glm::vec3 _position{};
    glm::vec3 _front{0.0f, 0.0f, -1.0f};
    glm::vec3 _up{0.0f, 1.0f, 0.0f};
    glm::vec3 _right;
    glm::vec3 _worldUp{0.0f, 1.0f, 0.0f};

    float _yaw = 0.0f;
    float _pitch = -55.0f;
};
} // namespace Game
