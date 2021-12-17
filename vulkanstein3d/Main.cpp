#include "Common.h"

#include "App/Input.h"
#include "App/Window.h"
#include "Game/Assets.h"
#include "Game/Components.h"
#include "Game/Intersection.h"
#include "Game/Level.h"
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

struct FrameConstantsUBO
{
    glm::mat4 view{1.0f};
    glm::mat4 projection{1.0f};
    float time;
    float mousexNormalized;
    float mouseyNormalized;
    float padding;
};

struct Sprite
{
    glm::mat4 model{1.0f};
    glm::vec4 data;
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

void CreateMaterials(std::shared_ptr<Rendering::Device> device, vk::RenderPass renderPass, Game::Assets& assets,
                     std::shared_ptr<Rendering::Buffer> frameUbo, std::shared_ptr<Rendering::Buffer> storage)
{
    // Hackyti-hax. Create layouts from reflected json files.

    //{
    //    std::vector bindings{vk::DescriptorSetLayoutBinding{0, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment, nullptr}};

    //    std::vector<vk::DescriptorBindingFlags> descriptorBindingFlags = {vk::DescriptorBindingFlagBits::eVariableDescriptorCount};
    //    const vk::DescriptorSetLayoutBindingFlagsCreateInfo descriptorSetLayoutBindingFlagsCreateInfo{descriptorBindingFlags};

    //    vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{{}, bindings};
    //    descriptorSetLayoutCreateInfo.pNext = &descriptorSetLayoutBindingFlagsCreateInfo;

    //    auto descriptorSetLayout = device->Get().createDescriptorSetLayout(descriptorSetLayoutCreateInfo).value;

    //    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts{descriptorSetLayout};
    //    std::vector<vk::PushConstantRange> pushConstantRanges{{vk::ShaderStageFlagBits::eAllGraphics, 0, sizeof(FrameConstants)}};

    //    auto pipelineLayout = device->Get().createPipelineLayout({{}, descriptorSetLayouts, pushConstantRanges}).value;

    //    auto testPipeline = Rendering::PipelineBuilder::Builder()
    //                            .SetRenderpass(renderPass)
    //                            .SetLayouts(pipelineLayout, descriptorSetLayout)
    //                            .SetShaders("Shaders/test.vert.spv", "Shaders/test.frag.spv")
    //                            .Build(device);

    //    materials["test"] = Rendering::MaterialBuilder::Builder()
    //                            .SetPipeline(testPipeline)
    //                            .SetTexture(assets.GetTexture("tex_walls"))
    //                            .Build(device);
    //}

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

        assets.AddMaterial("mat_map", Rendering::MaterialBuilder::Builder()
                                          .SetPipeline(testPipeline)
                                          .SetTexture(assets.GetTexture("tex_walls"))
                                          .Build(device));
    }

    {
        std::vector bindings{vk::DescriptorSetLayoutBinding{0, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment, nullptr},
                             vk::DescriptorSetLayoutBinding{1, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex, nullptr},
                             vk::DescriptorSetLayoutBinding{2, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eVertex, nullptr}};

        vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{{}, bindings};

        auto descriptorSetLayout = device->Get().createDescriptorSetLayout(descriptorSetLayoutCreateInfo).value;

        std::vector<vk::DescriptorSetLayout> descriptorSetLayouts{descriptorSetLayout};

        auto pipelineLayout = device->Get().createPipelineLayout({{}, descriptorSetLayouts, {}}).value;

        auto testPipeline = Rendering::PipelineBuilder::Builder()
                                .SetRenderpass(renderPass)
                                .SetLayouts(pipelineLayout, descriptorSetLayout)
                                .SetShaders("Shaders/mat_sprite.vert.spv", "Shaders/mat_sprite.frag.spv")
                                .SetBlend(true)
                                .Build(device);

        assets.AddMaterial("mat_sprites", Rendering::MaterialBuilder::Builder()
                                              .SetPipeline(testPipeline)
                                              .SetTexture(assets.GetTexture("tex_sprites"))
                                              .SetUboHack(frameUbo, storage)
                                              .Build(device));
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

        assets.AddMaterial("mat_ground", Rendering::MaterialBuilder::Builder()
                                             .SetPipeline(testPipeline2)
                                             .Build(device));
    }
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

    Wolf3dLoaders::Loaders loaders{dataPath};

    Game::Level level{loaders.LoadMap(1, 1)};

    auto& map = *level.GetMap();

    auto& registry = level.GetRegistry();

    auto& playert = registry.get<Game::Transform>(level.GetPlayerEntity());

    Game::PlayerController player{playert.position};

    playert.position.y = 5.5f;
    float cubeAngleRad = 0.0f;

    auto groundMesh = Game::MeshGenerator::BuildFloorPlaneMesh(device, map.width);
    auto mapMesh = Game::MeshGenerator::BuildMapMesh(device, map);
    auto cubeMesh = Game::MeshGenerator::BuildCubeMesh(device);

    std::vector<Sprite> spriteModelMats;

    auto frameConstsUbo = Rendering::Buffer::CreateUniformBuffer(device, sizeof(FrameConstantsUBO));
    auto modelStorage = Rendering::Buffer::CreateStorageBuffer(device, sizeof(Sprite) * 256);

    CreateMaterials(device, renderPass, assets, frameConstsUbo, modelStorage);
    
    auto groundMaterial = assets.GetMaterial("mat_ground");
    auto mapMaterial = assets.GetMaterial("mat_map");
    auto spriteMaterial = assets.GetMaterial("mat_sprites");

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
        level.Update(delta);

        spriteModelMats.clear();
        auto itemview = registry.view<Game::Transform, Game::Sprite>();
        for (auto [entity, itemtransform, csprite] : itemview.each())
        {
            Sprite sprite;
            sprite.model = glm::translate(glm::mat4{1.0f}, itemtransform.position);
            sprite.data = glm::vec4(csprite.spriteIndex);
            spriteModelMats.push_back(sprite);
        }
        modelStorage->SetData((void*)spriteModelMats.data(), sizeof(Sprite) * spriteModelMats.size());

        auto view = player.GetViewMatrix();
        auto proj = glm::perspective(glm::radians(45.0f), swapchain->GetExtent().width / (float)swapchain->GetExtent().height, 0.1f, 1000.0f);

        auto& input = App::Input::The();

        auto mousepos = input.GetMousePos();

        auto& playerTrans = registry.get<Game::Transform>(level.GetPlayerEntity());

        glm::vec3 cubeDir{glm::cos(cubeAngleRad), 0.0f, glm::sin(cubeAngleRad)};
        auto prevPos = playerTrans.position;
        glm::vec3 newPos = playerTrans.position;
        if (input.IsKeyDown(GLFW_KEY_UP))
            newPos = playerTrans.position + cubeDir * (float)delta * 50.0f;
        if (input.IsKeyDown(GLFW_KEY_DOWN))
            newPos = playerTrans.position - cubeDir * (float)delta * 50.0f;
        if (input.IsKeyDown(GLFW_KEY_LEFT))
            cubeAngleRad -= (float)delta * 5.0f;
        if (input.IsKeyDown(GLFW_KEY_RIGHT))
            cubeAngleRad += (float)delta * 5.0f;

        int tilex = (int)(newPos.x / 10.0f);
        int tiley = (int)(newPos.z / 10.0f);

        const auto& tiles = level.GetTiles();
        playerTrans.position.x = newPos.x;
        for (int y = tiley - 1; y < tiley + 2; y++)
        {
            for (int x = tilex - 1; x < tilex + 2; x++)
            {
                glm::vec4 rect{x * 10.0f + 5.0f, y * 10.0f + 5.0f, 10.0f, 10.0f};
                if ((tiles[y * 64 + x] & Game::TileBlocksMovement) && Game::Intersection::CircleRectIntersect({playerTrans.position.x, playerTrans.position.z}, 3.0f, rect))
                    playerTrans.position.x = prevPos.x;
            }
        }
        playerTrans.position.z = newPos.z;
        for (int y = tiley - 1; y < tiley + 2; y++)
        {
            for (int x = tilex - 1; x < tilex + 2; x++)
            {
                glm::vec4 rect{x * 10.0f + 5.0f, y * 10.0f + 5.0f, 10.0f, 10.0f};
                if ((tiles[y * 64 + x] & Game::TileBlocksMovement) && Game::Intersection::CircleRectIntersect({playerTrans.position.x, playerTrans.position.z}, 3.0f, rect))
                    playerTrans.position.z = prevPos.z;
            }
        }

        if (input.IsKeyDown(GLFW_KEY_TAB))
            view = glm::lookAt(playerTrans.position, playerTrans.position + cubeDir, {0.0f, 1.0f, 0.0f});

        FrameConstants consts{(float)totalTime, (float)mousepos.x / (float)swapchain->GetExtent().width, (float)mousepos.y / (float)swapchain->GetExtent().height, 0.0f};
        FrameConstantsUBO constsUbo{view, proj, (float)totalTime, (float)mousepos.x / (float)swapchain->GetExtent().width, (float)mousepos.y / (float)swapchain->GetExtent().height, 0.0f};

        frameConstsUbo->SetData((void*)&constsUbo, sizeof(FrameConstantsUBO));

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

        consts.mvp = proj * view * glm::translate(glm::mat4{1.0f}, playerTrans.position) * glm::rotate(glm::mat4{1.0f}, -cubeAngleRad, {0.0f, 1.0f, 0.0f}) * glm::scale(glm::mat4{1.0f}, glm::vec3{3.0f, 10.0f, 3.0f});
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, mapMaterial->_pipeline->pipeline);
        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, mapMaterial->_pipeline->pipelineLayout, 0, 1, &mapMaterial->_descriptorSet, 0, nullptr);
        commandBuffer.pushConstants(mapMaterial->_pipeline->pipelineLayout, vk::ShaderStageFlagBits::eAllGraphics, 0, sizeof(FrameConstants), &consts);
        vk::Buffer vertexBuffers3[] = {cubeMesh.vertexBuffer->Get()};
        commandBuffer.bindVertexBuffers(0, 1, vertexBuffers3, offsets);
        commandBuffer.bindIndexBuffer(cubeMesh.indexBuffer->Get(), 0, vk::IndexType::eUint32);
        commandBuffer.drawIndexed(cubeMesh.indexCount, 1, 0, 0, 0);

        auto rendeables = registry.view<Game::Transform, Game::Renderable>();
        for (auto [entity, rtransform, rmesh] : rendeables.each())
        {
            consts.mvp = proj * view * glm::translate(glm::mat4{ 1.0f }, rtransform.position) * glm::scale(glm::mat4{ 1.0f }, rtransform.scale);
            commandBuffer.pushConstants(mapMaterial->_pipeline->pipelineLayout, vk::ShaderStageFlagBits::eAllGraphics, 0, sizeof(FrameConstants), &consts);
            commandBuffer.drawIndexed(cubeMesh.indexCount, 1, 0, 0, 0);
        }

        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, spriteMaterial->_pipeline->pipeline);
        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, spriteMaterial->_pipeline->pipelineLayout, 0, 1, &spriteMaterial->_descriptorSet, 0, nullptr);
        commandBuffer.draw(6, spriteModelMats.size(), 0, 0);

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
