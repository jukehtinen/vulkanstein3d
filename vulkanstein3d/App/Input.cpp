#include "../Common.h"

#include "Input.h"
#include "Window.h"

namespace App
{
Input::Input()
{
}

void Input::Initialize(Window* window)
{
    glfwSetCursorPosCallback(window->Get(), [](GLFWwindow*, double x, double y) -> void {
        Input::The().SetMousePos({static_cast<int>(x), static_cast<int>(y)});
    });

    glfwSetKeyCallback(window->Get(), [](GLFWwindow*, int key, int scancode, int action, int mods) -> void {
        Input::The().SetKey(key, scancode, action, mods);
    });
}

void Input::Update()
{
    for (int i = 0; i < _buttons.size(); i++)
        _buttons[i] = 0;
}

void Input::SetKey(int key, int scancode, int action, int mods)
{
    _keys[key] = action == GLFW_RELEASE ? 0 : 1;

    if (key == GLFW_KEY_SPACE)
        _buttons[0] = action == GLFW_PRESS ? 1 : 0;
    if (key == GLFW_KEY_N)
        _buttons[1] = action == GLFW_PRESS ? 1 : 0;
}

Input& Input::The()
{
    static Input input{};

    return input;
}

} // namespace App