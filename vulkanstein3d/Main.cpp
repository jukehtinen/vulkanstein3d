#include "Common.h"

#include "App/Input.h"
#include "App/Window.h"
#include "Game/Assets.h"
#include "Game/MeshGenerator.h"
#include "Game/PlayerController.h"
#include "Rendering/Buffer.h"
#include "Rendering/Device.h"
#include "Rendering/Instance.h"
#include "Rendering/MaterialBuilder.h"
#include "Rendering/PipelineBuilder.h"
#include "Rendering/Swapchain.h"
#include "Rendering/Texture.h"
#include "Wolf3dLoaders/Loaders.h"

#include <chrono>
#include <map>

struct FrameConstants
{
    float time;
    float mousexNormalized;
    float mouseyNormalized;
    float padding;
    glm::mat4 mvp{1.0f};
};

std::vector<vk::Framebuffer> CreateFramebuffers(std::shared_ptr<Rendering::Device> device, std::shared_ptr<Rendering::Swapchain> swapchain, vk::RenderPass renderPass, std::shared_ptr<Rendering::Texture> depthTexture)
{
    auto& images = swapchain->GetImages();
    std::vector<vk::Framebuffer> frameBuffers;
    frameBuffers.reserve(images.size());
    for (auto& [image, imageView] : images)
    {
        std::vector attachments{imageView, depthTexture->_imageView};
        frameBuffers.push_back(device->Get().createFramebuffer({{}, renderPass, attachments, swapchain->GetExtent().width, swapchain->GetExtent().height, 1}).value);
    }
    return frameBuffers;
}

std::map<std::string, std::shared_ptr<Rendering::Material>> CreateMaterials(std::shared_ptr<Rendering::Device> device, vk::RenderPass renderPass, Game::Assets& assets)
{
    // Hackyti-hax. Create layouts from reflected json files.

    std::map<std::string, std::shared_ptr<Rendering::Material>> materials;

    {
        std::vector bindings{vk::DescriptorSetLayoutBinding{0, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment, nullptr}};

        std::vector<vk::DescriptorBindingFlags> descriptorBindingFlags = {vk::DescriptorBindingFlagBits::eVariableDescriptorCount};
        const vk::DescriptorSetLayoutBindingFlagsCreateInfo descriptorSetLayoutBindingFlagsCreateInfo{descriptorBindingFlags};

        vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{{}, bindings};
        descriptorSetLayoutCreateInfo.pNext = &descriptorSetLayoutBindingFlagsCreateInfo;

        auto descriptorSetLayout = device->Get().createDescriptorSetLayout(descriptorSetLayoutCreateInfo).value;

        std::vector<vk::DescriptorSetLayout> descriptorSetLayouts{descriptorSetLayout};
        std::vector<vk::PushConstantRange> pushConstantRanges{{vk::ShaderStageFlagBits::eAllGraphics, 0, sizeof(FrameConstants)}};

        auto pipelineLayout = device->Get().createPipelineLayout({{}, descriptorSetLayouts, pushConstantRanges}).value;

        auto testPipeline = Rendering::PipelineBuilder::Builder()
                                .SetRenderpass(renderPass)
                                .SetLayouts(pipelineLayout, descriptorSetLayout)
                                .SetShaders("Shaders/test.vert.spv", "Shaders/test.frag.spv")
                                .Build(device);

        materials["test"] = Rendering::MaterialBuilder::Builder()
                                .SetPipeline(testPipeline)
                                .SetTexture(assets._textures[0])
                                .Build(device);
    }

    {
        std::vector bindings{vk::DescriptorSetLayoutBinding{0, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment, nullptr}};

        vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{{}, bindings};

        auto descriptorSetLayout = device->Get().createDescriptorSetLayout(descriptorSetLayoutCreateInfo).value;

        std::vector<vk::DescriptorSetLayout> descriptorSetLayouts{descriptorSetLayout};
        std::vector<vk::PushConstantRange> pushConstantRanges{{vk::ShaderStageFlagBits::eAllGraphics, 0, sizeof(FrameConstants)}};

        std::vector vertexAttributes = {
            vk::VertexInputAttributeDescription{0, 0, vk::Format::eR32G32B32Sfloat, 0},
            vk::VertexInputAttributeDescription{1, 0, vk::Format::eR32G32B32Sfloat, sizeof(glm::vec3)},
            vk::VertexInputAttributeDescription{2, 0, vk::Format::eR32G32B32Sfloat, sizeof(glm::vec3) * 2}};

        std::vector vertexBinding = {vk::VertexInputBindingDescription{0, sizeof(glm::vec3) * 3, vk::VertexInputRate::eVertex}};

        auto pipelineLayout = device->Get().createPipelineLayout({{}, descriptorSetLayouts, pushConstantRanges}).value;

        auto testPipeline = Rendering::PipelineBuilder::Builder()
                                .SetRenderpass(renderPass)
                                .SetVertexInput(vertexBinding, vertexAttributes)
                                .SetLayouts(pipelineLayout, descriptorSetLayout)
                                .SetShaders("Shaders/mat_map.vert.spv", "Shaders/mat_map.frag.spv")
                                .Build(device);

        materials["map"] = Rendering::MaterialBuilder::Builder()
                               .SetPipeline(testPipeline)
                               .SetTexture(assets._textures[1])
                               .Build(device);
    }

    {
        std::vector<vk::PushConstantRange> pushConstantRanges{{vk::ShaderStageFlagBits::eAllGraphics, 0, sizeof(FrameConstants)}};

        std::vector vertexAttributes = {
            vk::VertexInputAttributeDescription{0, 0, vk::Format::eR32G32B32Sfloat, 0},
            vk::VertexInputAttributeDescription{1, 0, vk::Format::eR32G32B32Sfloat, sizeof(glm::vec3)},
            vk::VertexInputAttributeDescription{2, 0, vk::Format::eR32G32B32Sfloat, sizeof(glm::vec3) * 2}};

        std::vector vertexBinding = {vk::VertexInputBindingDescription{0, sizeof(glm::vec3) * 3, vk::VertexInputRate::eVertex}};

        auto pipelineLayout = device->Get().createPipelineLayout({{}, {}, pushConstantRanges}).value;

        auto testPipeline2 = Rendering::PipelineBuilder::Builder()
                                 .SetRenderpass(renderPass)
                                 .SetLayouts(pipelineLayout, {})
                                 .SetShaders("Shaders/mat_ground.vert.spv", "Shaders/mat_ground.frag.spv")
                                 .SetVertexInput(vertexBinding, vertexAttributes)
                                 .Build(device);

        materials["ground"] = Rendering::MaterialBuilder::Builder()
                                  .SetPipeline(testPipeline2)
                                  .Build(device);
    }
    return materials;
}

bool CircleRectIntersect(const glm::vec2& circle, float r, const glm::vec4& rect)
{
    auto circleDistancex = glm::abs(circle.x - rect.x);
    auto circleDistancey = glm::abs(circle.y - rect.y);

    if (circleDistancex > (rect.z / 2 + r))
        return false;
    if (circleDistancey > (rect.w / 2 + r))
        return false;

    if (circleDistancex <= (rect.z / 2))
        return true;

    if (circleDistancey <= (rect.w / 2))
        return true;

    auto cornerDistance_sq = glm::pow((circleDistancex - rect.z / 2), 2.0f) + glm::pow((circleDistancey - rect.w / 2), 2.0f);

    return (cornerDistance_sq <= glm::pow(r, 2.0f));
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

    const std::vector attachments{
        vk::AttachmentDescription{{}, swapchain->GetFormat(), vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR},
        vk::AttachmentDescription{{}, vk::Format::eD32Sfloat, vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eDontCare, vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal}};

    const std::vector colorAttachmentRefs{vk::AttachmentReference{0, vk::ImageLayout::eColorAttachmentOptimal}};
    const vk::AttachmentReference depthAttachmentRef{1, vk::ImageLayout::eDepthStencilAttachmentOptimal};

    const std::vector subpassDescriptions{vk::SubpassDescription{{}, vk::PipelineBindPoint::eGraphics, {}, colorAttachmentRefs, {}, &depthAttachmentRef, {}}};

    auto renderPass = dev.createRenderPass({{}, attachments, subpassDescriptions, {}}).value;

    auto depthTexture = Rendering::Texture::CreateDepthTexture(device, swapchain->GetExtent().width, swapchain->GetExtent().height);
    auto frameBuffers = CreateFramebuffers(device, swapchain, renderPass, depthTexture);

    Game::Assets assets{device, dataPath};

    auto materials = CreateMaterials(device, renderPass, assets);

    auto testMaterial = materials["test"];
    auto groundMaterial = materials["ground"];
    auto mapMaterial = materials["map"];

    Wolf3dLoaders::Loaders loaders{dataPath};
    auto map = loaders.LoadMap(1, 1);

    Game::PlayerController player{glm::vec3{map.playerStart % map.width * 10.0f, 35.5f, map.playerStart / map.width * 10.0f}};

    glm::vec3 cubePos{map.playerStart % map.width * 10.0f + 5.0f, 5.5f, map.playerStart / map.width * 10.0f + 5.0f};
    float cubeAngleRad = 0.0f;    

    auto groundMesh = Game::MeshGenerator::BuildFloorPlaneMesh(device, map.width);
    auto mapMesh = Game::MeshGenerator::BuildMapMesh(device, map);
    auto cubeMesh = Game::MeshGenerator::BuildCubeMesh(device);

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

            depthTexture = Rendering::Texture::CreateDepthTexture(device, swapchain->GetExtent().width, swapchain->GetExtent().height);

            frameBuffers = CreateFramebuffers(device, swapchain, renderPass, depthTexture);
        }

        auto nowTime = std::chrono::high_resolution_clock::now();
        auto timeSpan = std::chrono::duration_cast<std::chrono::duration<double>>(nowTime - prevTime);
        prevTime = nowTime;
        auto delta = timeSpan.count();
        totalTime += delta;

        player.Update(static_cast<float>(delta));

        auto& input = App::Input::The();
        auto mousepos = input.GetMousePos();
        FrameConstants consts{(float)totalTime, (float)mousepos.x / (float)swapchain->GetExtent().width, (float)mousepos.y / (float)swapchain->GetExtent().height, 0.0f};

        glm::vec3 cubeDir{glm::cos(cubeAngleRad), 0.0f, glm::sin(cubeAngleRad)};
        auto prevPos = cubePos;
        glm::vec3 newPos = cubePos;
        if (input.IsKeyDown(GLFW_KEY_UP))
            newPos = cubePos + cubeDir * (float)delta * 50.0f;
        if (input.IsKeyDown(GLFW_KEY_DOWN))
            newPos = cubePos - cubeDir * (float)delta * 50.0f;
        if (input.IsKeyDown(GLFW_KEY_LEFT))
            cubeAngleRad -= (float)delta * 5.0f;
        if (input.IsKeyDown(GLFW_KEY_RIGHT))
            cubeAngleRad += (float)delta * 5.0f;

        cubePos.x = newPos.x;
        for (int i = 0; i < map.tiles[0].size(); i++)
        {
            glm::vec4 rect{i % map.width * 10.0f + 5.0f, i / map.width * 10.0f + 5.0f, 10.0f, 10.0f};
            if (map.tiles[0][i] < 108 && CircleRectIntersect({cubePos.x, cubePos.z}, 3.0f, rect))
                cubePos.x = prevPos.x;
        }
        cubePos.z = newPos.z;
        for (int i = 0; i < map.tiles[0].size(); i++)
        {
            glm::vec4 rect{i % map.width * 10.0f + 5.0f, i / map.width * 10.0f + 5.0f, 10.0f, 10.0f};
            if (map.tiles[0][i] < 108 && CircleRectIntersect({cubePos.x, cubePos.z}, 3.0f, rect))
                cubePos.z = prevPos.z;
        }

        dev.waitForFences(1, &renderFence, VK_TRUE, UINT64_MAX);
        dev.resetFences(1, &renderFence);

        uint32_t imageIndex = dev.acquireNextImageKHR(swapchain->Get(), UINT64_MAX, presentSemaphore).value;

        commandBuffer.reset({});

        commandBuffer.begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});

        std::vector clearValues{vk::ClearValue{vk::ClearColorValue{std::array<float, 4>{0.22f, 0.22f, 0.22f, 1.0f}}},
                                vk::ClearValue{vk::ClearDepthStencilValue{1.0f}}};

        vk::RenderPassBeginInfo renderPassBeginInfo{};
        renderPassBeginInfo.renderPass = renderPass;
        renderPassBeginInfo.renderArea.offset.x = 0;
        renderPassBeginInfo.renderArea.offset.y = 0;
        renderPassBeginInfo.renderArea.extent = swapchain->GetExtent();
        renderPassBeginInfo.framebuffer = frameBuffers[imageIndex];
        renderPassBeginInfo.setClearValues(clearValues);

        commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

        const glm::vec4 viewArea{0.0f, 0.0f, swapchain->GetExtent().width, swapchain->GetExtent().height};
        const auto viewportPost = vk::Viewport{viewArea.x, viewArea.w - viewArea.y, viewArea.z, -viewArea.w, 0.0f, 1.0f};
        const auto scissor = vk::Rect2D{{0, 0}, swapchain->GetExtent()};

        commandBuffer.setScissor(0, 1, &scissor);
        commandBuffer.setViewport(0, 1, &viewportPost);

        /*commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, testMaterial->_pipeline->pipeline);
        commandBuffer.pushConstants(testMaterial->_pipeline->pipelineLayout, vk::ShaderStageFlagBits::eAllGraphics, 0, sizeof(FrameConstants), &consts);
        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, testMaterial->_pipeline->pipelineLayout, 0, 1, &testMaterial->_descriptorSet, 0, nullptr);
        commandBuffer.draw(3, 1, 0, 0);*/        

        auto view = player.GetViewMatrix();
        auto proj = glm::perspective(glm::radians(45.0f), swapchain->GetExtent().width / (float)swapchain->GetExtent().height, 0.1f, 1000.0f);

        if (input.IsKeyDown(GLFW_KEY_TAB))
            view = glm::lookAt(cubePos, cubePos + cubeDir, {0.0f, 1.0f, 0.0f});

        consts.mvp = proj * view * glm::scale(glm::mat4{1.0f}, glm::vec3{10.0f});
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, groundMaterial->_pipeline->pipeline);
        //commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, groundMaterial->_pipeline->pipelineLayout, 0, 1, &groundMaterial->_descriptorSet, 0, nullptr);
        commandBuffer.pushConstants(groundMaterial->_pipeline->pipelineLayout, vk::ShaderStageFlagBits::eAllGraphics, 0, sizeof(FrameConstants), &consts);
        vk::Buffer vertexBuffers[] = {groundMesh.vertexBuffer->Get()};
        vk::DeviceSize offsets[] = {0};
        commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);
        commandBuffer.bindIndexBuffer(groundMesh.indexBuffer->Get(), 0, vk::IndexType::eUint32);
        commandBuffer.drawIndexed(groundMesh.indexCount, 1, 0, 0, 0);

        consts.mvp = proj * view * glm::scale(glm::mat4{1.0f}, glm::vec3{10.0f}) * glm::translate(glm::mat4{1.0f}, glm::vec3{0.5, 0.0f, 0.5f});
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, mapMaterial->_pipeline->pipeline);
        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, mapMaterial->_pipeline->pipelineLayout, 0, 1, &mapMaterial->_descriptorSet, 0, nullptr);
        commandBuffer.pushConstants(mapMaterial->_pipeline->pipelineLayout, vk::ShaderStageFlagBits::eAllGraphics, 0, sizeof(FrameConstants), &consts);
        vk::Buffer vertexBuffers2[] = {mapMesh.vertexBuffer->Get()};
        commandBuffer.bindVertexBuffers(0, 1, vertexBuffers2, offsets);
        commandBuffer.bindIndexBuffer(mapMesh.indexBuffer->Get(), 0, vk::IndexType::eUint32);
        commandBuffer.drawIndexed(mapMesh.indexCount, 1, 0, 0, 0);

        consts.mvp = proj * view * glm::translate(glm::mat4{1.0f}, cubePos) * glm::rotate(glm::mat4{1.0f}, -cubeAngleRad, {0.0f, 1.0f, 0.0f}) * glm::scale(glm::mat4{1.0f}, glm::vec3{3.0f, 10.0f, 3.0f});
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, mapMaterial->_pipeline->pipeline);
        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, mapMaterial->_pipeline->pipelineLayout, 0, 1, &mapMaterial->_descriptorSet, 0, nullptr);
        commandBuffer.pushConstants(mapMaterial->_pipeline->pipelineLayout, vk::ShaderStageFlagBits::eAllGraphics, 0, sizeof(FrameConstants), &consts);
        vk::Buffer vertexBuffers3[] = {cubeMesh.vertexBuffer->Get()};
        commandBuffer.bindVertexBuffers(0, 1, vertexBuffers3, offsets);
        commandBuffer.bindIndexBuffer(cubeMesh.indexBuffer->Get(), 0, vk::IndexType::eUint32);
        commandBuffer.drawIndexed(cubeMesh.indexCount, 1, 0, 0, 0);

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
    for (auto& fb : frameBuffers)
        dev.destroyFramebuffer(fb);
    dev.destroyRenderPass(renderPass);

    return 0;
}
