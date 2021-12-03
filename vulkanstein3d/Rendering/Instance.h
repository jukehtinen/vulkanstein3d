#pragma once

namespace App
{
class Window;
}

namespace Rendering
{
class Instance
{
  public:
    static std::shared_ptr<Instance> CreateInstance(std::shared_ptr<App::Window> window);

    Instance(vk::Instance instance, vk::SurfaceKHR surface, vk::DebugUtilsMessengerEXT debugMessenger, vk::DynamicLoader& dynamicLoader);
    ~Instance();

    vk::Instance Get() const { return _instance; }
    vk::SurfaceKHR GetSurface() const { return _surface; }

  private:
    vk::Instance _instance{};
    vk::SurfaceKHR _surface{};
    vk::DebugUtilsMessengerEXT _debugMessenger{};
    vk::DynamicLoader _dynamicLoader{};
};
} // namespace Rendering