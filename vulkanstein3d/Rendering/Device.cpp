#include "../Common.h"

#include "Device.h"
#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#include <vector>

namespace Rendering
{
static const std::vector<const char*> requiredDeviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_MAINTENANCE1_EXTENSION_NAME,
    VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME};

Device::Device(vk::PhysicalDevice physicalDevice, vk::Device device, vk::Queue graphicsQueue, VmaAllocator allocator)
    : _physicalDevice(physicalDevice), _device(device), _graphicsQueue(graphicsQueue), _allocator(allocator)
{
}

Device::~Device()
{
    _device.destroy();
}

void Device::RunCommandsSync(std::function<void(vk::CommandBuffer)> func)
{
    auto commandPool = _device.createCommandPool({vk::CommandPoolCreateFlagBits::eResetCommandBuffer}).value;
    auto commandBuffer = _device.allocateCommandBuffers({commandPool, vk::CommandBufferLevel::ePrimary, 1}).value.front();

    commandBuffer.begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
    func(commandBuffer);
    commandBuffer.end();

    const vk::CommandBufferSubmitInfoKHR cmdBufferSubmit{commandBuffer};
    const vk::SubmitInfo2KHR submitInfo{{}, 0, nullptr, 1, &cmdBufferSubmit, 0, nullptr};
    _graphicsQueue.submit2KHR(1, &submitInfo, {});
    _graphicsQueue.waitIdle();

    _device.destroyCommandPool(commandPool);
}

static bool ValidateRequirements(vk::PhysicalDevice physicalDevice)
{
    const auto [result, extensions] = physicalDevice.enumerateDeviceExtensionProperties();

    if (result != vk::Result::eSuccess)
    {
        spdlog::error("[Vulkan] enumerateDeviceExtensionProperties: {}", vk::to_string(result));
        return false;
    }

    for (const auto ext : requiredDeviceExtensions)
    {
        if (std::find_if(extensions.begin(), extensions.end(), [ext](const vk::ExtensionProperties& e) {
                return std::strcmp(ext, e.extensionName) == 0;
            }) == extensions.end())
        {
            spdlog::error("[Vulkan] Extension '{}' not found.", ext);
            return false;
        }
    }

    return true;
}

std::shared_ptr<Device> Device::CreateDevice(std::shared_ptr<Instance> instance)
{
    const auto [result, physicalDevices] = instance->Get().enumeratePhysicalDevices();

    if (result != vk::Result::eSuccess)
    {
        spdlog::error("[Vulkan] vk::enumeratePhysicalDevices: {}", vk::to_string(result));
        return nullptr;
    }

    vk::PhysicalDevice physicalDevice{};

    auto discreteGpu = std::find_if(physicalDevices.begin(), physicalDevices.end(), [](const vk::PhysicalDevice& pd) {
        return pd.getProperties().deviceType == vk::PhysicalDeviceType::eDiscreteGpu;
    });

    if (discreteGpu != physicalDevices.end())
        physicalDevice = *discreteGpu;
    else
        physicalDevice = physicalDevices.front();

    if (!ValidateRequirements(physicalDevice))
        return nullptr;

    const auto props = physicalDevice.getProperties();

    spdlog::debug("[Vulkan] Using device '{0}', type '{1}'", props.deviceName, vk::to_string(props.deviceType));

    const auto queues = physicalDevice.getQueueFamilyProperties();
    uint32_t graphicsQueueIndex = 0;
    for (auto i = 0; i < queues.size(); i++)
    {
        if (queues[i].queueFlags & vk::QueueFlagBits::eGraphics &&
            physicalDevice.getSurfaceSupportKHR(i, instance->GetSurface()).value == VK_TRUE)
        {
            graphicsQueueIndex = i;
            break;
        }
    }

    std::vector queuePriorities{1.0f};
    std::vector deviceQueueInfos{vk::DeviceQueueCreateInfo{{}, graphicsQueueIndex, queuePriorities}};

    vk::PhysicalDeviceSynchronization2FeaturesKHR synchronization2Features{};
    vk::PhysicalDeviceDescriptorIndexingFeatures descriptorIndexingFeatures{};
    vk::PhysicalDeviceDynamicRenderingFeaturesKHR dynamicRenderingFeaturesKHR{};
    vk::PhysicalDeviceFeatures2 physicalDeviceFeatures2{};

    descriptorIndexingFeatures.setPNext(&dynamicRenderingFeaturesKHR);
    synchronization2Features.setPNext(&descriptorIndexingFeatures);
    physicalDeviceFeatures2.setPNext(&synchronization2Features);

    physicalDevice.getFeatures2(&physicalDeviceFeatures2);

    synchronization2Features.setSynchronization2(true);
    dynamicRenderingFeaturesKHR.setDynamicRendering(true);

    vk::DeviceCreateInfo deviceCreateInfo{{}, deviceQueueInfos, {}, requiredDeviceExtensions, {}};
    deviceCreateInfo.setPNext(&synchronization2Features);

    const auto [deviceResult, device] = physicalDevice.createDevice(deviceCreateInfo);

    if (deviceResult != vk::Result::eSuccess)
    {
        spdlog::error("[Vulkan] createDevice: {}", vk::to_string(result));
        return nullptr;
    }

    auto graphicsQueue = device.getQueue(graphicsQueueIndex, 0);

    VmaAllocatorCreateInfo allocatorInfo{};
    allocatorInfo.physicalDevice = physicalDevice;
    allocatorInfo.device = device;
    allocatorInfo.instance = instance->Get();
    VmaAllocator allocator{};
    vmaCreateAllocator(&allocatorInfo, &allocator);

    return std::make_shared<Device>(physicalDevice, device, graphicsQueue, allocator);
}

} // namespace Rendering