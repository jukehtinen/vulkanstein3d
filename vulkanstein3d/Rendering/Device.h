#pragma once

#include "Instance.h"

namespace Rendering
{
class Device
{
  public:
    static std::shared_ptr<Device> CreateDevice(std::shared_ptr<Instance> instance);

    Device(vk::PhysicalDevice physicalDevice, vk::Device device, vk::Queue graphicQueue, VmaAllocator allocator);
    ~Device();

    vk::Device Get() const { return _device; }
    vk::PhysicalDevice GetPhysicalDevice() const { return _physicalDevice; }
    vk::Queue GetGraphicQueue() const { return _graphicQueue; }
    VmaAllocator GetAllocator() const { return _allocator; }

  private:
    vk::PhysicalDevice _physicalDevice{};
    vk::Device _device{};
    vk::Queue _graphicQueue{};
    VmaAllocator _allocator{};
};
} // namespace Rendering