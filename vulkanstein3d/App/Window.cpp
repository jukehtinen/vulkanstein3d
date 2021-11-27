#include "Window.h"
#include "Input.h"

#include "spdlog/spdlog.h"

namespace App
{
Window::Window(int width, int height)
    : _width(width), _height(height)
{
    glfwSetErrorCallback(
        [](int code, const char* message) { spdlog::error("[glfw] Error: '{}', Message: '{}'", code, message); });
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    _window = glfwCreateWindow(width, height, "Vulkanstein3d", nullptr, nullptr);
    if (_window == nullptr)
    {
        glfwTerminate();
    }

    glfwSetWindowUserPointer(_window, this);

    glfwSetWindowSizeCallback(_window, [](GLFWwindow* wnd, int width, int height) {});

    App::Input::The().Initialize(this);
}

Window::~Window()
{
    glfwDestroyWindow(_window);
    glfwTerminate();
}

vk::SurfaceKHR Window::CreateSurface(vk::Instance instance)
{
    vk::SurfaceKHR surface{};
    auto result = (vk::Result)glfwCreateWindowSurface(instance, _window, nullptr, (VkSurfaceKHR*)&surface);
    if (result != vk::Result::eSuccess)
    {
        spdlog::error("[glfw] glfwCreateWindowSurface: {}", vk::to_string(result));
        return {};
    }
    return surface;
}

} // namespace App
