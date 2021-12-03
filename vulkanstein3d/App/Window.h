#pragma once

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