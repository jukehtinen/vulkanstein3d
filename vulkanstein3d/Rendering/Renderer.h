#pragma once

#include "Buffer.h"
#include "Device.h"
#include "Instance.h"
#include "MaterialBuilder.h"
#include "Mesh.h"
#include "PipelineBuilder.h"
#include "Swapchain.h"
#include "Texture.h"

namespace Rendering
{
class Renderer
{
  public:
    Renderer(std::shared_ptr<App::Window> window)
    {
        _instance = Rendering::Instance::CreateInstance(window);
        _device = Rendering::Device::CreateDevice(_instance);
        _swapchain = Rendering::Swapchain::CreateSwapchain(_device, _instance->GetSurface());
        _depthTexture = Rendering::Texture::CreateDepthTexture(_device, _swapchain->GetExtent().width, _swapchain->GetExtent().height);
    }

    ~Renderer()
    {
    }

    std::shared_ptr<Rendering::Instance> _instance;
    std::shared_ptr<Rendering::Device> _device;
    std::shared_ptr<Rendering::Swapchain> _swapchain;
    std::shared_ptr<Rendering::Texture> _depthTexture;
};
} // namespace Rendering