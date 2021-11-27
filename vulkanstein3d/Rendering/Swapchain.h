#pragma once

#include "Device.h"

namespace Rendering
{
class Swapchain
{
  public:
    static std::shared_ptr<Swapchain> CreateSwapchain(std::shared_ptr<Device> device, vk::SurfaceKHR surface);

    Swapchain(std::shared_ptr<Device> device, vk::SwapchainKHR swapchain, vk::SwapchainCreateInfoKHR swapchainCreateInfo);
    ~Swapchain();

    bool RefreshSwapchain();

    vk::SwapchainKHR Get() const { return _swapchain; }
    vk::Format GetFormat() const { return _swapchainCreateInfo.imageFormat; }
    vk::Extent2D GetExtent() { return _swapchainCreateInfo.imageExtent; }
    std::vector<std::pair<vk::Image, vk::ImageView>>& GetImages() { return _swapChainImages; }

  private:
    std::shared_ptr<Device> _device;
    vk::SwapchainKHR _swapchain{};
    std::vector<std::pair<vk::Image, vk::ImageView>> _swapChainImages;
    vk::SwapchainCreateInfoKHR _swapchainCreateInfo{};
};
} // namespace Rendering