#include "App/Input.h"
#include "App/Window.h"
#include "Game/Assets.h"
#include "Rendering/Device.h"
#include "Rendering/Instance.h"
#include "Rendering/MaterialBuilder.h"
#include "Rendering/PipelineBuilder.h"
#include "Rendering/Swapchain.h"
#include "Rendering/Texture.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/scalar_constants.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <chrono>

struct FrameConstants
{
    float time;
    float mousexNormalized;
    float mouseyNormalized;
    float padding;
};

std::vector<vk::Framebuffer> CreateFramebuffers(std::shared_ptr<Rendering::Device> device, std::shared_ptr<Rendering::Swapchain> swapchain, vk::RenderPass renderPass)
{
    auto& images = swapchain->GetImages();
    std::vector<vk::Framebuffer> frameBuffers;
    frameBuffers.reserve(images.size());
    for (auto& [image, imageView] : images)
    {
        frameBuffers.push_back(device->Get().createFramebuffer({{}, renderPass, 1, &imageView, swapchain->GetExtent().width, swapchain->GetExtent().height, 1}).value);
    }
    return frameBuffers;
}

int main(int argc, char* argv[])
{
    spdlog::set_level(spdlog::level::debug);

    if (argc < 2)
    {
        spdlog::warn("Pass path to Wolf3D directory as an argument.");
        return 1;
    }

    std::filesystem::path dataPath = argv[1];

    auto window = std::make_shared<App::Window>();
    auto instance = Rendering::Instance::CreateInstance(window);
    auto device = Rendering::Device::CreateDevice(instance);
    auto swapchain = Rendering::Swapchain::CreateSwapchain(device, instance->GetSurface());

    auto dev = device->Get();

    const std::vector colorAttachments{vk::AttachmentDescription{{}, swapchain->GetFormat(), vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR}};
    const std::vector colorAttachmentRefs{vk::AttachmentReference{0, vk::ImageLayout::eColorAttachmentOptimal}};
    const std::vector subpassDescriptions{vk::SubpassDescription{{}, vk::PipelineBindPoint::eGraphics, {}, colorAttachmentRefs, {}, nullptr, {}}};

    auto renderPass = dev.createRenderPass({{}, colorAttachments, subpassDescriptions, {}}).value;

    auto frameBuffers = CreateFramebuffers(device, swapchain, renderPass);

    Game::Assets assets{device, dataPath};

    auto testPipeline = Rendering::PipelineBuilder::Builder()
                            .SetRenderpass(renderPass)
                            .SetShaders("Shaders/test.vert.spv", "Shaders/test.frag.spv")
                            .SetPushConstants(sizeof(FrameConstants))
                            .Build(device);

    auto testMaterial = Rendering::MaterialBuilder::Builder()
                            .SetPipeline(testPipeline)
                            .SetTexture(assets._textures[0])
                            .Build(device);

    vk::Fence renderFence = dev.createFence({vk::FenceCreateFlagBits::eSignaled}).value;
    vk::Semaphore presentSemaphore = dev.createSemaphore({}).value;
    vk::Semaphore renderSemaphore = dev.createSemaphore({}).value;

    auto commandPool = dev.createCommandPool({vk::CommandPoolCreateFlagBits::eResetCommandBuffer}).value;
    auto commandBuffer = dev.allocateCommandBuffers({commandPool, vk::CommandBufferLevel::ePrimary, 1}).value.front();

    bool recreateSwapchain = false;
    auto prevTime = std::chrono::high_resolution_clock::now();
    double totalTime{};
    while (!glfwWindowShouldClose(window->Get()))
    {
        glfwPollEvents();

        if (recreateSwapchain)
        {
            device->Get().waitIdle();
            if (!swapchain->RefreshSwapchain())
            {
                glfwWaitEvents();
                continue;
            }

            recreateSwapchain = false;

            for (auto& fb : frameBuffers)
                dev.destroyFramebuffer(fb);

            frameBuffers = CreateFramebuffers(device, swapchain, renderPass);
        }

        auto nowTime = std::chrono::high_resolution_clock::now();
        auto timeSpan = std::chrono::duration_cast<std::chrono::duration<double>>(nowTime - prevTime);
        prevTime = nowTime;
        auto delta = timeSpan.count();
        totalTime += delta;

        auto mousepos = App::Input::The().GetMousePos();
        FrameConstants consts{(float)totalTime, (float)mousepos.x / (float)swapchain->GetExtent().width, (float)mousepos.y / (float)swapchain->GetExtent().height, 0.0f};

        dev.waitForFences(1, &renderFence, VK_TRUE, UINT64_MAX);
        dev.resetFences(1, &renderFence);

        uint32_t imageIndex = dev.acquireNextImageKHR(swapchain->Get(), UINT64_MAX, presentSemaphore).value;

        commandBuffer.reset({});

        commandBuffer.begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});

        std::vector clearValues{vk::ClearValue{vk::ClearColorValue{std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f}}}};

        vk::RenderPassBeginInfo renderPassBeginInfo{};
        renderPassBeginInfo.renderPass = renderPass;
        renderPassBeginInfo.renderArea.offset.x = 0;
        renderPassBeginInfo.renderArea.offset.y = 0;
        renderPassBeginInfo.renderArea.extent = swapchain->GetExtent();
        renderPassBeginInfo.framebuffer = frameBuffers[imageIndex];
        renderPassBeginInfo.setClearValues(clearValues);

        commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

        const glm::vec4 viewArea{0.0f, 0.0f, swapchain->GetExtent().width, swapchain->GetExtent().height};
        const auto viewportPost = vk::Viewport{viewArea.x, viewArea.y, viewArea.z, viewArea.w, 0.0f, 1.0f};
        const auto scissor = vk::Rect2D{{0, 0}, swapchain->GetExtent()};

        commandBuffer.setScissor(0, 1, &scissor);
        commandBuffer.setViewport(0, 1, &viewportPost);

        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, testPipeline.pipeline);
        commandBuffer.pushConstants(testPipeline.pipelineLayout, vk::ShaderStageFlagBits::eAllGraphics, 0, sizeof(FrameConstants), &consts);
        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, testMaterial.pipeline.pipelineLayout, 0, 1, &testMaterial._descriptorSet, 0, nullptr);
        commandBuffer.draw(3, 1, 0, 0);

        commandBuffer.endRenderPass();

        commandBuffer.end();

        const vk::CommandBufferSubmitInfoKHR cmdBufferSubmit{commandBuffer};
        const vk::SemaphoreSubmitInfoKHR waitSemaphore{presentSemaphore, 0, vk::PipelineStageFlagBits2KHR::eAllCommands, 0};
        const vk::SemaphoreSubmitInfoKHR signalSemaphore{renderSemaphore, 0, vk::PipelineStageFlagBits2KHR::eAllCommands, 0};

        const vk::SubmitInfo2KHR submitInfo{{}, 1, &waitSemaphore, 1, &cmdBufferSubmit, 1, &signalSemaphore};

        auto graphicsQueue = device->GetGraphicQueue();
        graphicsQueue.submit2KHR(1, &submitInfo, renderFence);

        auto sc = swapchain->Get();
        const vk::PresentInfoKHR presentInfo{1, &renderSemaphore, 1, &sc, &imageIndex};
        auto presentResult = graphicsQueue.presentKHR(presentInfo);
        if (presentResult != vk::Result::eSuccess)
        {
            spdlog::warn("[Vulkan] presentKHR: {}", vk::to_string(presentResult));
            recreateSwapchain = true;
        }
    }

    dev.waitIdle();

    dev.destroyCommandPool(commandPool);
    dev.destroySemaphore(renderSemaphore);
    dev.destroySemaphore(presentSemaphore);
    dev.destroyFence(renderFence);
    dev.destroyPipeline(testPipeline.pipeline);
    dev.destroyPipelineLayout(testPipeline.pipelineLayout);
    for (auto& fb : frameBuffers)
        dev.destroyFramebuffer(fb);
    dev.destroyRenderPass(renderPass);

    return 0;
}
