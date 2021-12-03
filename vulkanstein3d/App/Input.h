#pragma once

namespace App
{
class Window;

class Input
{
  public:
    static Input& The();

    void Initialize(Window* window);

    glm::ivec2 GetMousePos() const { return _mousePos; }
    void SetMousePos(const glm::ivec2& pos) { _mousePos = pos; }

  private:
    Input();

    glm::ivec2 _mousePos{};
};
} // namespace App