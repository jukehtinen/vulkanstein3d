#include "Common.h"

#include "PlayerController.h"

#include "App/Input.h"

namespace Game
{
const float MaxSpeed = 55.0f;
const float Sensitivity = 0.1f;
const float FOV = 45.0f;

PlayerController::PlayerController(const glm::vec3& pos)
    : _position(pos)
{
    UpdateVectors();
}

void PlayerController::Update(float delta)
{
    auto& input = App::Input::The();

    float velocity = MaxSpeed * delta;
    if (input.IsKeyDown(GLFW_KEY_W))
        _position += _front * velocity;
    if (input.IsKeyDown(GLFW_KEY_S))
        _position -= _front * velocity;
    if (input.IsKeyDown(GLFW_KEY_A))
        _position -= _right * velocity;
    if (input.IsKeyDown(GLFW_KEY_D))
        _position += _right * velocity;

    if (input.IsKeyDown(GLFW_KEY_Q))
        _yaw -= velocity * 20.0f;
    if (input.IsKeyDown(GLFW_KEY_E))
        _yaw += velocity * 20.0f;

    if (input.IsKeyDown(GLFW_KEY_SPACE))
        _position += glm::vec3(0.0f, 1.0f, 0.0f) * velocity;
    if (input.IsKeyDown(GLFW_KEY_C))
        _position += glm::vec3(0.0f, -1.0f, 0.0f) * velocity;

    UpdateVectors();
}

const glm::mat4 PlayerController::GetViewMatrix()
{
    return glm::lookAt(_position, _position + _front, _up);
}

void PlayerController::UpdateVectors()
{
    glm::vec3 front{glm::cos(glm::radians(_yaw)) * glm::cos(glm::radians(_pitch)), glm::sin(glm::radians(_pitch)), glm::sin(glm::radians(_yaw)) * glm::cos(glm::radians(_pitch))};
    _front = glm::normalize(front);
    _right = glm::normalize(glm::cross(_front, _worldUp));
    _up = glm::normalize(glm::cross(_right, _front));
}

} // namespace Game