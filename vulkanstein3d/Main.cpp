#include "Common.h"

#include "App/Input.h"
#include "App/Window.h"
#include "Game/Assets.h"
#include "Game/Components.h"
#include "Game/Intersection.h"
#include "Game/Level.h"
#include "Game/MeshGenerator.h"
#include "Game/PlayerController.h"
#include "Rendering/Renderer.h"
#include "Wolf3dLoaders/Loaders.h"

#include <chrono>
#include <map>

struct HudPushConstants
{
    glm::mat4 vp;
    glm::vec2 scale;
    glm::vec2 translate;
    int textureIndex{0};
};

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

void CreateMaterials(std::shared_ptr<Rendering::Device> device, Game::Assets& assets,
                     std::shared_ptr<Rendering::Buffer> frameUbo, std::shared_ptr<Rendering::Buffer> storage)
{

    auto hudPipeline = Rendering::PipelineBuilder::Builder()
                           .SetDepthState(false, false)
                           .SetRasterization(vk::CullModeFlagBits::eNone, vk::FrontFace::eCounterClockwise)
                           .SetShaders("Shaders/mat_hud.vert.spv", "Shaders/mat_hud.frag.spv")
                           .Build(device);

    assets.AddMaterial("mat_hud_loading", Rendering::MaterialBuilder::Builder()
                                              .SetPipeline(hudPipeline)
                                              .SetTexture(0, assets.GetTexture("tex_gui_loading"))
                                              .Build(device));

    assets.AddMaterial("mat_hud_intro", Rendering::MaterialBuilder::Builder()
                                            .SetPipeline(hudPipeline)
                                            .SetTexture(0, assets.GetTexture("tex_gui_intro"))
                                            .Build(device));

    assets.AddMaterial("mat_hud_weapons", Rendering::MaterialBuilder::Builder()
                                              .SetPipeline(hudPipeline)
                                              .SetTexture(0, assets.GetTexture("tex_gui_weapons"))
                                              .Build(device));

    assets.AddMaterial("mat_hud_keys", Rendering::MaterialBuilder::Builder()
                                           .SetPipeline(hudPipeline)
                                           .SetTexture(0, assets.GetTexture("tex_gui_keys"))
                                           .Build(device));

    auto mapPipeline = Rendering::PipelineBuilder::Builder()
                           .SetShaders("Shaders/mat_map.vert.spv", "Shaders/mat_map.frag.spv")
                           .Build(device);

    assets.AddMaterial("mat_map", Rendering::MaterialBuilder::Builder()
                                      .SetPipeline(mapPipeline)
                                      .SetTexture(0, assets.GetTexture("tex_walls"))
                                      .Build(device));

    auto spritePipeline = Rendering::PipelineBuilder::Builder()
                              .SetShaders("Shaders/mat_sprite.vert.spv", "Shaders/mat_sprite.frag.spv")
                              .SetBlend(true)
                              .Build(device);

    assets.AddMaterial("mat_sprites", Rendering::MaterialBuilder::Builder()
                                          .SetPipeline(spritePipeline)
                                          .SetTexture(0, assets.GetTexture("tex_sprites"))
                                          .SetBuffer(1, frameUbo)
                                          .SetBuffer(2, storage)
                                          .Build(device));

    auto groundPipeline = Rendering::PipelineBuilder::Builder()
                              .SetShaders("Shaders/mat_ground.vert.spv", "Shaders/mat_ground.frag.spv")
                              .Build(device);

    assets.AddMaterial("mat_ground", Rendering::MaterialBuilder::Builder()
                                         .SetPipeline(groundPipeline)
                                         .Build(device));
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
    Rendering::Renderer renderer{window};

    Game::Assets assets{renderer._device, dataPath};
    Wolf3dLoaders::Loaders loaders{dataPath};

    auto map = loaders.LoadMap(1, 1);

    Game::Level level{renderer, map};

    auto& registry = level.GetRegistry();

    auto& playert = registry.get<Game::Transform>(level.GetPlayerEntity());

    Game::PlayerController player{playert.position};

    playert.position.y = 5.5f;
    float cubeAngleRad = 0.0f;

    auto cubeMesh = Game::MeshGenerator::BuildCubeMesh(renderer._device);

    std::vector<Sprite> spriteModelMats;

    auto frameConstsUbo = Rendering::Buffer::CreateUniformBuffer(renderer._device, sizeof(FrameConstantsUBO));
    auto modelStorage = Rendering::Buffer::CreateStorageBuffer(renderer._device, sizeof(Sprite) * 256);

    CreateMaterials(renderer._device, assets, frameConstsUbo, modelStorage);

    auto groundMaterial = assets.GetMaterial("mat_ground");
    auto mapMaterial = assets.GetMaterial("mat_map");
    auto spriteMaterial = assets.GetMaterial("mat_sprites");
    auto hudMaterial = assets.GetMaterial("mat_hud_weapons");

    auto device = renderer._device;
    auto dev = renderer._device->Get();

    auto prevTime = std::chrono::high_resolution_clock::now();
    double totalTime{};
    while (!glfwWindowShouldClose(window->Get()))
    {
        glfwPollEvents();

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
            sprite.data = glm::vec4((float)csprite.spriteIndex);
            spriteModelMats.push_back(sprite);
        }
        modelStorage->SetData((void*)spriteModelMats.data(), sizeof(Sprite) * spriteModelMats.size());

        auto view = player.GetViewMatrix();
        auto proj = glm::perspective(glm::radians(45.0f), renderer._swapchain->GetExtent().width / (float)renderer._swapchain->GetExtent().height, 0.1f, 1000.0f);

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

        FrameConstants consts{(float)totalTime, (float)mousepos.x / (float)renderer._swapchain->GetExtent().width, (float)mousepos.y / (float)renderer._swapchain->GetExtent().height, 0.0f};
        FrameConstantsUBO constsUbo{view, proj, (float)totalTime, (float)mousepos.x / (float)renderer._swapchain->GetExtent().width, (float)mousepos.y / (float)renderer._swapchain->GetExtent().height, 0.0f};

        frameConstsUbo->SetData((void*)&constsUbo, sizeof(FrameConstantsUBO));

        if (!renderer.Begin())
        {
            // Couldn't begin rendering (window hidden, swapchain borked, etc.), try again later.
            glfwWaitEvents();
            continue;
        }

        consts.mvp = proj * view * glm::scale(glm::mat4{1.0f}, glm::vec3{10.0f});
        renderer._commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, groundMaterial->_pipeline->pipeline);
        renderer._commandBuffer.pushConstants(groundMaterial->_pipeline->pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(FrameConstants), &consts);
        vk::Buffer vertexBuffers[] = {level._floorMesh._vertexBuffer->Get()};
        vk::DeviceSize offsets[] = {0};
        renderer._commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);
        renderer._commandBuffer.bindIndexBuffer(level._floorMesh._indexBuffer->Get(), 0, vk::IndexType::eUint32);
        renderer._commandBuffer.drawIndexed(level._floorMesh._indexCount, 1, 0, 0, 0);

        consts.mvp = proj * view * glm::scale(glm::mat4{1.0f}, glm::vec3{10.0f}) * glm::translate(glm::mat4{1.0f}, glm::vec3{0.5, 0.0f, 0.5f});
        renderer._commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, mapMaterial->_pipeline->pipeline);
        renderer._commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, mapMaterial->_pipeline->pipelineLayout, 0, 1, &mapMaterial->_descriptorSet, 0, nullptr);
        renderer._commandBuffer.pushConstants(mapMaterial->_pipeline->pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(FrameConstants), &consts);
        vk::Buffer vertexBuffers2[] = {level._mapMesh._vertexBuffer->Get()};
        renderer._commandBuffer.bindVertexBuffers(0, 1, vertexBuffers2, offsets);
        renderer._commandBuffer.bindIndexBuffer(level._mapMesh._indexBuffer->Get(), 0, vk::IndexType::eUint32);
        renderer._commandBuffer.drawIndexed(level._mapMesh._indexCount, 1, 0, 0, 0);

        consts.mvp = proj * view * glm::translate(glm::mat4{1.0f}, playerTrans.position) * glm::rotate(glm::mat4{1.0f}, -cubeAngleRad, {0.0f, 1.0f, 0.0f}) * glm::scale(glm::mat4{1.0f}, glm::vec3{3.0f, 10.0f, 3.0f});
        renderer._commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, mapMaterial->_pipeline->pipeline);
        renderer._commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, mapMaterial->_pipeline->pipelineLayout, 0, 1, &mapMaterial->_descriptorSet, 0, nullptr);
        renderer._commandBuffer.pushConstants(mapMaterial->_pipeline->pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(FrameConstants), &consts);
        vk::Buffer vertexBuffers3[] = {cubeMesh._vertexBuffer->Get()};
        renderer._commandBuffer.bindVertexBuffers(0, 1, vertexBuffers3, offsets);
        renderer._commandBuffer.bindIndexBuffer(cubeMesh._indexBuffer->Get(), 0, vk::IndexType::eUint32);
        renderer._commandBuffer.drawIndexed(cubeMesh._indexCount, 1, 0, 0, 0);

        auto rendeables = registry.view<Game::Transform, Game::Renderable>();
        for (auto [entity, rtransform, rmesh] : rendeables.each())
        {
            consts.mvp = proj * view * glm::translate(glm::mat4{1.0f}, rtransform.position) * glm::scale(glm::mat4{1.0f}, rtransform.scale);
            renderer._commandBuffer.pushConstants(mapMaterial->_pipeline->pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(FrameConstants), &consts);
            renderer._commandBuffer.drawIndexed(cubeMesh._indexCount, 1, 0, 0, 0);
        }

        renderer._commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, spriteMaterial->_pipeline->pipeline);
        renderer._commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, spriteMaterial->_pipeline->pipelineLayout, 0, 1, &spriteMaterial->_descriptorSet, 0, nullptr);
        renderer._commandBuffer.draw(6, spriteModelMats.size(), 0, 0);

        // Hud
        auto orthoMat = glm::ortho(0.0f, (float)renderer._swapchain->GetExtent().width, (float)renderer._swapchain->GetExtent().height, 0.0f);

        glm::vec2 weaponSize{48, 24};
        HudPushConstants hudPushConstants{orthoMat, weaponSize * 4.0f, {250.0f, 250.0f}, 1};

        renderer._commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, hudMaterial->_pipeline->pipeline);
        renderer._commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, hudMaterial->_pipeline->pipelineLayout, 0, 1, &hudMaterial->_descriptorSet, 0, nullptr);
        renderer._commandBuffer.pushConstants(hudMaterial->_pipeline->pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(HudPushConstants), &hudPushConstants);
        renderer._commandBuffer.draw(6, 1, 0, 0);

        renderer.End();
    }

    dev.waitIdle();

    return 0;
}
