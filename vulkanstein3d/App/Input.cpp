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
}

Input& Input::The()
{
    static Input input{};

    return input;
}

} // namespace App