#include "Renderer.h"

#include "../Common.h"

namespace Rendering
{
Renderer::Renderer(std::shared_ptr<App::Window> window)
{
    _instance = Rendering::Instance::CreateInstance(window);
    _device = Rendering::Device::CreateDevice(_instance);
    _swapchain = Rendering::Swapchain::CreateSwapchain(_device, _instance->GetSurface());
    _depthTexture = Rendering::Texture::CreateDepthTexture(_device, _swapchain->GetExtent().width, _swapchain->GetExtent().height);

    auto dev = _device->Get();

    _commandPool = dev.createCommandPool({vk::CommandPoolCreateFlagBits::eResetCommandBuffer}).value;
    _commandBuffer = dev.allocateCommandBuffers({_commandPool, vk::CommandBufferLevel::ePrimary, 1}).value.front();

    _renderFence = dev.createFence({vk::FenceCreateFlagBits::eSignaled}).value;
    _presentSemaphore = dev.createSemaphore({}).value;
    _renderSemaphore = dev.createSemaphore({}).value;
}

Renderer::~Renderer()
{
    auto dev = _device->Get();

    auto idleResult = dev.waitIdle();

    dev.destroySemaphore(_renderSemaphore);
    dev.destroySemaphore(_presentSemaphore);
    dev.destroyFence(_renderFence);

    dev.destroyCommandPool(_commandPool);
}

bool Renderer::Begin()
{
    auto dev = _device->Get();

    if (_recreateSwapchain)
    {
        auto idleResult = dev.waitIdle();
        if (idleResult != vk::Result::eSuccess || !_swapchain->RefreshSwapchain())
        {
            return false;
        }

        _recreateSwapchain = false;
        _depthTexture = Rendering::Texture::CreateDepthTexture(_device, _swapchain->GetExtent().width, _swapchain->GetExtent().height);
    }

    auto fenceResult = dev.waitForFences(1, &_renderFence, VK_TRUE, UINT64_MAX);
    auto resetResult = dev.resetFences(1, &_renderFence);

    _imageIndex = dev.acquireNextImageKHR(_swapchain->Get(), UINT64_MAX, _presentSemaphore).value;

    _commandBuffer.reset({});
    auto beginResult = _commandBuffer.begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
    if (beginResult != vk::Result::eSuccess)
    {
        spdlog::warn("[Vulkan] begin: {}", vk::to_string(beginResult));
    }

    // Swapchain image -> eColorAttachmentOptimal
    // Depth image -> eColorAttachmentOptimal
    std::vector<vk::ImageMemoryBarrier2KHR> attachmentBarriers(2);
    attachmentBarriers[0].setImage(_swapchain->GetImages()[_imageIndex].first);
    attachmentBarriers[0].setSubresourceRange(vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
    attachmentBarriers[0].setOldLayout(vk::ImageLayout::eUndefined);
    attachmentBarriers[0].setNewLayout(vk::ImageLayout::eColorAttachmentOptimal);
    attachmentBarriers[0].setSrcStageMask(vk::PipelineStageFlagBits2KHR::eTopOfPipe);
    attachmentBarriers[0].setDstStageMask(vk::PipelineStageFlagBits2KHR::eColorAttachmentOutput);
    attachmentBarriers[0].setSrcAccessMask(vk::AccessFlagBits2KHR::eNone);
    attachmentBarriers[0].setDstAccessMask(vk::AccessFlagBits2KHR::eColorAttachmentWrite);
    attachmentBarriers[1].setImage(_depthTexture->_image);
    attachmentBarriers[1].setSubresourceRange(vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1});
    attachmentBarriers[1].setOldLayout(vk::ImageLayout::eUndefined);
    attachmentBarriers[1].setNewLayout(vk::ImageLayout::eDepthAttachmentOptimal);
    attachmentBarriers[1].setSrcStageMask(vk::PipelineStageFlagBits2KHR::eEarlyFragmentTests | vk::PipelineStageFlagBits2KHR::eLateFragmentTests);
    attachmentBarriers[1].setDstStageMask(vk::PipelineStageFlagBits2KHR::eEarlyFragmentTests | vk::PipelineStageFlagBits2KHR::eLateFragmentTests);
    attachmentBarriers[1].setSrcAccessMask(vk::AccessFlagBits2KHR::eNone);
    attachmentBarriers[1].setDstAccessMask(vk::AccessFlagBits2KHR::eDepthStencilAttachmentWrite);
    _commandBuffer.pipelineBarrier2KHR(vk::DependencyInfoKHR{{}, {}, {}, attachmentBarriers});

    BeginRenderPass();

    return true;
}

void Renderer::End()
{
    EndRenderPass();

    // Swapchain image -> ePresentSrcKHR
    vk::ImageMemoryBarrier2KHR attachmentToPresentBarrier{};
    attachmentToPresentBarrier.setImage(_swapchain->GetImages()[_imageIndex].first);
    attachmentToPresentBarrier.setSubresourceRange(vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
    attachmentToPresentBarrier.setOldLayout(vk::ImageLayout::eColorAttachmentOptimal);
    attachmentToPresentBarrier.setNewLayout(vk::ImageLayout::ePresentSrcKHR);
    attachmentToPresentBarrier.setSrcStageMask(vk::PipelineStageFlagBits2KHR::eColorAttachmentOutput);
    attachmentToPresentBarrier.setDstStageMask(vk::PipelineStageFlagBits2KHR::eBottomOfPipe);
    attachmentToPresentBarrier.setSrcAccessMask(vk::AccessFlagBits2KHR::eColorAttachmentWrite);
    attachmentToPresentBarrier.setDstAccessMask({});
    _commandBuffer.pipelineBarrier2KHR(vk::DependencyInfoKHR{{}, 0, nullptr, 0, nullptr, 1, &attachmentToPresentBarrier});

    auto endResult = _commandBuffer.end();
    if (endResult != vk::Result::eSuccess)
    {
        spdlog::warn("[Vulkan] begin: {}", vk::to_string(endResult));
    }

    Submit();
}

void Renderer::BeginRenderPass()
{
    vk::RenderingAttachmentInfoKHR colorAttachment{};
    colorAttachment.setClearValue(vk::ClearValue{vk::ClearColorValue{std::array<float, 4>{0.22f, 0.22f, 0.22f, 1.0f}}});
    colorAttachment.setLoadOp(vk::AttachmentLoadOp::eClear);
    colorAttachment.setStoreOp(vk::AttachmentStoreOp::eStore);
    colorAttachment.setResolveMode(vk::ResolveModeFlagBits::eNone);
    colorAttachment.setImageLayout(vk::ImageLayout::eAttachmentOptimalKHR);
    colorAttachment.setImageView(_swapchain->GetImages()[_imageIndex].second);

    vk::RenderingAttachmentInfoKHR depthAttachment{};
    depthAttachment.setClearValue(vk::ClearValue{vk::ClearDepthStencilValue{1.0f}});
    depthAttachment.setLoadOp(vk::AttachmentLoadOp::eClear);
    depthAttachment.setStoreOp(vk::AttachmentStoreOp::eDontCare);
    depthAttachment.setResolveMode(vk::ResolveModeFlagBits::eNone);
    depthAttachment.setImageLayout(vk::ImageLayout::eDepthAttachmentOptimal);
    depthAttachment.setImageView(_depthTexture->_imageView);

    vk::RenderingInfoKHR renderingInfo{};
    renderingInfo.setPColorAttachments(&colorAttachment);
    renderingInfo.setColorAttachmentCount(1);
    renderingInfo.setPDepthAttachment(&depthAttachment);
    renderingInfo.setRenderArea(vk::Rect2D{{0, 0}, _swapchain->GetExtent()});
    renderingInfo.setLayerCount(1);

    _commandBuffer.beginRenderingKHR(renderingInfo);

    const glm::vec4 viewArea{0.0f, 0.0f, _swapchain->GetExtent().width, _swapchain->GetExtent().height};
    const auto viewportPost = vk::Viewport{viewArea.x, viewArea.w - viewArea.y, viewArea.z, -viewArea.w, 0.0f, 1.0f};
    const auto scissor = vk::Rect2D{{0, 0}, _swapchain->GetExtent()};

    _commandBuffer.setScissor(0, 1, &scissor);
    _commandBuffer.setViewport(0, 1, &viewportPost);
}

void Renderer::EndRenderPass()
{
    _commandBuffer.endRenderingKHR();
}

void Renderer::Submit()
{
    const vk::CommandBufferSubmitInfoKHR cmdBufferSubmit{_commandBuffer};
    const vk::SemaphoreSubmitInfoKHR waitSemaphore{_presentSemaphore, 0, vk::PipelineStageFlagBits2KHR::eAllCommands, 0};
    const vk::SemaphoreSubmitInfoKHR signalSemaphore{_renderSemaphore, 0, vk::PipelineStageFlagBits2KHR::eAllCommands, 0};

    const vk::SubmitInfo2KHR submitInfo{{}, 1, &waitSemaphore, 1, &cmdBufferSubmit, 1, &signalSemaphore};

    auto graphicsQueue = _device->GetGraphicQueue();
    auto submitResult = graphicsQueue.submit2KHR(1, &submitInfo, _renderFence);
    if (submitResult != vk::Result::eSuccess)
    {
        spdlog::warn("[Vulkan] submit2KHR: {}", vk::to_string(submitResult));
    }

    auto sc = _swapchain->Get();
    const vk::PresentInfoKHR presentInfo{1, &_renderSemaphore, 1, &sc, &_imageIndex};
    auto presentResult = graphicsQueue.presentKHR(presentInfo);
    if (presentResult != vk::Result::eSuccess)
    {
        spdlog::warn("[Vulkan] presentKHR: {}", vk::to_string(presentResult));
        _recreateSwapchain = true;
    }
}

void Renderer::Draw(uint32_t vertexCount, uint32_t instances, std::shared_ptr<Rendering::Material> material, void* pushConstants, size_t pushConstantSize)
{
    _commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, material->_pipeline->pipeline);

    if (material->_descriptorSet)
        _commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, material->_pipeline->pipelineLayout, 0, 1, &material->_descriptorSet, 0, nullptr);

    if (pushConstants)
        _commandBuffer.pushConstants(material->_pipeline->pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, (uint32_t)pushConstantSize, pushConstants);

    _commandBuffer.draw(vertexCount, instances, 0, 0);
}

void Renderer::DrawMesh(Rendering::Mesh& mesh, std::shared_ptr<Rendering::Material> material, void* pushConstants, size_t pushConstantSize)
{
    vk::Buffer vertexBuffers[] = {mesh._vertexBuffer->Get()};
    vk::DeviceSize offsets[] = {0};

    _commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, material->_pipeline->pipeline);

    if (material->_descriptorSet)
        _commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, material->_pipeline->pipelineLayout, 0, 1, &material->_descriptorSet, 0, nullptr);

    if (pushConstants)
        _commandBuffer.pushConstants(material->_pipeline->pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, (uint32_t)pushConstantSize, pushConstants);

    _commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);
    _commandBuffer.bindIndexBuffer(mesh._indexBuffer->Get(), 0, vk::IndexType::eUint32);

    _commandBuffer.drawIndexed(mesh._indexCount, 1, 0, 0, 0);
}

} // namespace Rendering