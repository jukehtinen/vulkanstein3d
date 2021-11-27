#include "Instance.h"
#include "../App/Window.h"

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE;

namespace Rendering
{
static const std::vector<const char*> requiredExtensions = {
    VK_KHR_SURFACE_EXTENSION_NAME,
    VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#ifdef VULKAN_DEBUG
    VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif
};

static const std::vector<const char*> requiredValidationLayers = {
#ifdef VULKAN_DEBUG
    "VK_LAYER_KHRONOS_validation", "VK_LAYER_KHRONOS_synchronization2"
#endif
};

Instance::Instance(vk::Instance instance, vk::SurfaceKHR surface, vk::DebugUtilsMessengerEXT debugMessenger, vk::DynamicLoader& dynamicLoader)
    : _instance(instance), _surface(surface), _debugMessenger(debugMessenger), _dynamicLoader(std::move(dynamicLoader))
{
}

Instance::~Instance()
{
    _instance.destroySurfaceKHR(_surface);
    _instance.destroyDebugUtilsMessengerEXT(_debugMessenger);
    _instance.destroy();
}

static bool ValidateRequirements()
{
    const auto [extResult, extensionProps] = vk::enumerateInstanceExtensionProperties();
    if (extResult != vk::Result::eSuccess)
    {
        spdlog::error("[Vulkan] vk::enumerateInstanceExtensionProperties: {}", vk::to_string(extResult));
        return false;
    }

    for (const auto ext : requiredExtensions)
    {
        if (std::find_if(extensionProps.begin(), extensionProps.end(), [ext](const vk::ExtensionProperties& e) {
                return std::strcmp(ext, e.extensionName) == 0;
            }) == extensionProps.end())
        {
            spdlog::error("[Vulkan] Extension '{}' not found.", ext);
            return false;
        }
    }

    const auto [layerResult, layerProps] = vk::enumerateInstanceLayerProperties();

    if (layerResult != vk::Result::eSuccess)
    {
        spdlog::error("[Vulkan] vk::enumerateInstanceLayerProperties: {}", vk::to_string(layerResult));
        return false;
    }

    for (const auto layer : requiredValidationLayers)
    {
        if (std::find_if(layerProps.begin(), layerProps.end(), [layer](const vk::LayerProperties& l) {
                return std::strcmp(layer, l.layerName) == 0;
            }) == layerProps.end())
        {
            spdlog::error("[Vulkan] Extension '{}' not found.", layer);
            return false;
        }
    }

    return true;
}

static vk::DebugUtilsMessengerEXT CreateDebugCallback(vk::Instance instance)
{
#ifdef VULKAN_DEBUG

    static auto messengerCallback =
        [](VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType,
           const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) -> VkBool32 {
        bool isError =
            messageSeverity & VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        spdlog::log(isError ? spdlog::level::err : spdlog::level::warn, pCallbackData->pMessage);
        return messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    };

    vk::DebugUtilsMessengerCreateInfoEXT debugMessengerInfo{
        {},
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
            vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation,
        messengerCallback};

    return instance.createDebugUtilsMessengerEXT(debugMessengerInfo).value;
#elif
    return {};
#endif
}

std::shared_ptr<Instance> Instance::CreateInstance(std::shared_ptr<App::Window> window)
{
    vk::DynamicLoader dynamicLoader{};
    VULKAN_HPP_DEFAULT_DISPATCHER.init(
        dynamicLoader.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr"));

    if (!ValidateRequirements())
        return nullptr;

    vk::ApplicationInfo applicationInfo{nullptr, 1, nullptr, 1, VK_API_VERSION_1_2};
    vk::InstanceCreateInfo instanceCreateInfo{{}, &applicationInfo, requiredValidationLayers, requiredExtensions};

    auto [result, instance] = vk::createInstance(instanceCreateInfo);

    if (result != vk::Result::eSuccess)
    {
        spdlog::error("[Vulkan] vk::createInstance: {}", vk::to_string(result));
        return nullptr;
    }

    VULKAN_HPP_DEFAULT_DISPATCHER.init(instance);

    auto debugMessenger = CreateDebugCallback(instance);

    auto surface = window->CreateSurface(instance);

    return std::make_shared<Instance>(instance, surface, debugMessenger, dynamicLoader);
}

} // namespace Rendering