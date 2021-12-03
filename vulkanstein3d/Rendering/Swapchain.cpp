#include "../Common.h"

#include "Swapchain.h"

namespace Rendering
{
Swapchain::Swapchain(std::shared_ptr<Device> device, vk::SwapchainKHR swapchain, vk::SwapchainCreateInfoKHR swapchainCreateInfo)
    : _device(device), _swapchain(swapchain), _swapchainCreateInfo(swapchainCreateInfo)
{
    auto images = _device->Get().getSwapchainImagesKHR(swapchain).value;

    _swapChainImages.reserve(images.size());
    for (auto& image : images)
    {
        vk::ImageViewCreateInfo imageViewCreateInfo{{}, image, vk::ImageViewType::e2D, _swapchainCreateInfo.imageFormat, {}, {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}};
        auto [result, imageView] = _device->Get().createImageView(imageViewCreateInfo);

        if (result != vk::Result::eSuccess)
        {
            spdlog::error("[Vulkan] createImageView: {}", vk::to_string(result));
        }

        _swapChainImages.push_back(std::make_pair(image, imageView));
    }
}

Swapchain::~Swapchain()
{
    for (auto [image, imageView] : _swapChainImages)
        _device->Get().destroyImageView(imageView);

    _device->Get().destroySwapchainKHR(_swapchain);
}

bool Swapchain::RefreshSwapchain()
{
    auto physicalDevice = _device->GetPhysicalDevice();
    const auto surfaceCaps = physicalDevice.getSurfaceCapabilitiesKHR(_swapchainCreateInfo.surface).value;

    auto extents = surfaceCaps.currentExtent;
    if (extents.height == 0 || extents.width == 0)
        return false;

    _swapchainCreateInfo.setImageExtent(extents);
    _swapchainCreateInfo.setOldSwapchain(_swapchain);

    auto [result, swapchain] = _device->Get().createSwapchainKHR(_swapchainCreateInfo);

    if (result != vk::Result::eSuccess)
    {
        spdlog::error("[Vulkan] RefreshSwapchain: {}", vk::to_string(result));
    }

    _device->Get().destroySwapchainKHR(_swapchain);

    _swapchain = swapchain;

    auto images = _device->Get().getSwapchainImagesKHR(swapchain).value;
    _swapChainImages.clear();
    for (auto& image : images)
    {
        vk::ImageViewCreateInfo imageViewCreateInfo{{}, image, vk::ImageViewType::e2D, _swapchainCreateInfo.imageFormat, {}, {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}};
        auto [result, imageView] = _device->Get().createImageView(imageViewCreateInfo);

        if (result != vk::Result::eSuccess)
        {
            spdlog::error("[Vulkan] createImageView: {}", vk::to_string(result));
            return false;
        }

        _swapChainImages.push_back(std::make_pair(image, imageView));
    }

    return true;
}

std::shared_ptr<Swapchain> Swapchain::CreateSwapchain(std::shared_ptr<Device> device, vk::SurfaceKHR surface)
{
    auto physicalDevice = device->GetPhysicalDevice();

    const auto surfaceCaps = physicalDevice.getSurfaceCapabilitiesKHR(surface).value;
    const auto formats = physicalDevice.getSurfaceFormatsKHR(surface).value;
    const auto presentation = physicalDevice.getSurfacePresentModesKHR(surface).value;

    auto extents = surfaceCaps.currentExtent;
    if (extents.height == 0 || extents.width == 0)
    {
        return nullptr;
    }

    vk::SurfaceFormatKHR surfaceFormat{};

    auto formatIter = std::find_if(formats.begin(), formats.end(), [](const vk::SurfaceFormatKHR& f) {
        return f.format == vk::Format::eB8G8R8A8Unorm && f.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear;
    });

    if (formatIter != formats.end())
        surfaceFormat = *formatIter;
    else
        surfaceFormat = formats.front();

    vk::SwapchainCreateInfoKHR swapchainCreateInfoKHR{};
    swapchainCreateInfoKHR.setImageFormat(surfaceFormat.format);
    swapchainCreateInfoKHR.setImageColorSpace(surfaceFormat.colorSpace);
    swapchainCreateInfoKHR.setImageExtent(extents);
    swapchainCreateInfoKHR.setImageArrayLayers(1);
    swapchainCreateInfoKHR.setMinImageCount(surfaceCaps.minImageCount + 1);
    swapchainCreateInfoKHR.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);
    swapchainCreateInfoKHR.setImageSharingMode(vk::SharingMode::eExclusive);
    swapchainCreateInfoKHR.setPreTransform(surfaceCaps.currentTransform);
    swapchainCreateInfoKHR.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);
    swapchainCreateInfoKHR.setClipped(true);
    swapchainCreateInfoKHR.setPresentMode(vk::PresentModeKHR::eFifo);
    swapchainCreateInfoKHR.setSurface(surface);

    auto [result, swapchain] = device->Get().createSwapchainKHR(swapchainCreateInfoKHR);

    if (result != vk::Result::eSuccess)
    {
        spdlog::error("[Vulkan] createSwapchainKHR: {}", vk::to_string(result));
        return nullptr;
    }

    return std::make_shared<Swapchain>(device, swapchain, swapchainCreateInfoKHR);
}
} // namespace Rendering