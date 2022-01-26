#pragma once

#include <array>

namespace App
{
class Window;

class Input
{
  public:
    static Input& The();

    void Initialize(Window* window);

    void Update();

    glm::ivec2 GetMousePos() const { return _mousePos; }
    bool IsKeyDown(int key) const { return _keys[key] == 1; }

    bool IsButtonPressed(int button) const { return _buttons[button] == 1; }

  private:
    Input();

    void SetKey(int key, int scancode, int action, int mods);
    void SetMousePos(const glm::ivec2& pos) { _mousePos = pos; }

    glm::ivec2 _mousePos{};
    std::array<int, GLFW_KEY_LAST> _keys;
    std::array<int, 4> _buttons;
};
} // namespace App