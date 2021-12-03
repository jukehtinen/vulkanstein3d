#pragma once

#include "Instance.h"

namespace Rendering
{
class Device
{
  public:
    static std::shared_ptr<Device> CreateDevice(std::shared_ptr<Instance> instance);

    Device(vk::PhysicalDevice physicalDevice, vk::Device device, vk::Queue graphicsQueue, VmaAllocator allocator);
    ~Device();

    vk::Device Get() const { return _device; }
    vk::PhysicalDevice GetPhysicalDevice() const { return _physicalDevice; }
    vk::Queue GetGraphicQueue() const { return _graphicsQueue; }
    VmaAllocator GetAllocator() const { return _allocator; }

    void RunCommandsSync(std::function<void(vk::CommandBuffer)> func);

  private:
    vk::PhysicalDevice _physicalDevice{};
    vk::Device _device{};
    vk::Queue _graphicsQueue{};
    VmaAllocator _allocator{};
};
} // namespace Rendering