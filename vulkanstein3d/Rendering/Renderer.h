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

        auto dev = _device->Get();

        // Renderpass
        const std::vector attachments{
            vk::AttachmentDescription{{}, _swapchain->GetFormat(), vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR},
            vk::AttachmentDescription{{}, vk::Format::eD32Sfloat, vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eDontCare, vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal}};

        const std::vector colorAttachmentRefs{vk::AttachmentReference{0, vk::ImageLayout::eColorAttachmentOptimal}};
        const vk::AttachmentReference depthAttachmentRef{1, vk::ImageLayout::eDepthStencilAttachmentOptimal};

        const std::vector subpassDescriptions{vk::SubpassDescription{{}, vk::PipelineBindPoint::eGraphics, {}, colorAttachmentRefs, {}, &depthAttachmentRef, {}}};

        _renderPass = dev.createRenderPass({{}, attachments, subpassDescriptions, {}}).value;

        _depthTexture = Rendering::Texture::CreateDepthTexture(_device, _swapchain->GetExtent().width, _swapchain->GetExtent().height);
        _frameBuffers = CreateFramebuffers(_device, _swapchain, _renderPass, _depthTexture);
    }

    ~Renderer()
    {
        for (auto& fb : _frameBuffers)
            _device->Get().destroyFramebuffer(fb);
        _device->Get().destroyRenderPass(_renderPass);
    }

    std::vector<vk::Framebuffer> CreateFramebuffers(std::shared_ptr<Rendering::Device> device, std::shared_ptr<Rendering::Swapchain> swapchain, vk::RenderPass renderPass, std::shared_ptr<Rendering::Texture> depthTexture)
    {
        auto& images = swapchain->GetImages();
        std::vector<vk::Framebuffer> frameBuffers;
        frameBuffers.reserve(images.size());
        for (auto& [image, imageView] : images)
        {
            std::vector attachments{ imageView, depthTexture->_imageView };
            frameBuffers.push_back(device->Get().createFramebuffer({ {}, renderPass, attachments, swapchain->GetExtent().width, swapchain->GetExtent().height, 1 }).value);
        }
        return frameBuffers;
    }

    std::shared_ptr<Rendering::Instance> _instance;
    std::shared_ptr<Rendering::Device> _device;
    std::shared_ptr<Rendering::Swapchain> _swapchain;

    vk::RenderPass _renderPass{};
    std::vector<vk::Framebuffer> _frameBuffers;

    std::shared_ptr<Rendering::Texture> _depthTexture;
};
} // namespace Rendering