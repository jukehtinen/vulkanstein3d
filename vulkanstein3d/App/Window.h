#pragma once

#define VULKAN_HPP_NO_EXCEPTIONS
#define VULKAN_HPP_ASSERT_ON_RESULT
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include "vulkan/vulkan.hpp"

#include "GLFW/glfw3.h"
#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/glfw3native.h"
#endif

namespace App
{
class Window
{
  public:
    Window(int width = 1280, int height = 720);
    ~Window();

    GLFWwindow* Get() { return _window; }

    vk::SurfaceKHR CreateSurface(vk::Instance instance);

  private:
    GLFWwindow* _window{nullptr};
    int _width{0};
    int _height{0};
};
} // namespace App