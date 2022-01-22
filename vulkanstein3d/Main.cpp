#include "Common.h"

#include "App/Input.h"
#include "App/Window.h"
#include "Game/Assets.h"
#include "Game/Components.h"
#include "Game/Intersection.h"
#include "Game/Level.h"
#include "Game/MeshGenerator.h"
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

    assets.AddMaterial("mat_hud_sprites", Rendering::MaterialBuilder::Builder()
                                              .SetPipeline(hudPipeline)
                                              .SetTexture(0, assets.GetTexture("tex_sprites"))
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

    auto cubeMesh = Game::MeshGenerator::BuildCubeMesh(renderer._device);

    std::vector<Sprite> spriteModelMats;

    auto frameConstsUbo = Rendering::Buffer::CreateUniformBuffer(renderer._device, sizeof(FrameConstantsUBO));
    auto modelStorage = Rendering::Buffer::CreateStorageBuffer(renderer._device, sizeof(Sprite) * 256);

    CreateMaterials(renderer._device, assets, frameConstsUbo, modelStorage);

    auto groundMaterial = assets.GetMaterial("mat_ground");
    auto mapMaterial = assets.GetMaterial("mat_map");
    auto spriteMaterial = assets.GetMaterial("mat_sprites");
    auto hudMaterial = assets.GetMaterial("mat_hud_sprites");

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

        auto& input = App::Input::The();

        auto mousepos = input.GetMousePos();

        auto view = level._playerController.GetViewMatrix();
        auto proj = glm::perspective(glm::radians(45.0f), renderer._swapchain->GetExtent().width / (float)renderer._swapchain->GetExtent().height, 0.1f, 1000.0f);

        FrameConstants consts{(float)totalTime, (float)mousepos.x / (float)renderer._swapchain->GetExtent().width, (float)mousepos.y / (float)renderer._swapchain->GetExtent().height, 0.0f};
        FrameConstantsUBO constsUbo{view, proj, (float)totalTime, (float)mousepos.x / (float)renderer._swapchain->GetExtent().width, (float)mousepos.y / (float)renderer._swapchain->GetExtent().height, 0.0f};

        frameConstsUbo->SetData((void*)&constsUbo, sizeof(FrameConstantsUBO));

        if (!renderer.Begin())
        {
            // Couldn't begin rendering (window hidden, swapchain borked, etc.), try again later.
            glfwWaitEvents();
            continue;
        }

        vk::DeviceSize offsets[] = {0};

        // Draw map
        consts.mvp = proj * view * glm::scale(glm::mat4{1.0f}, glm::vec3{10.0f}) * glm::translate(glm::mat4{1.0f}, glm::vec3{0.5, 0.0f, 0.5f});
        renderer.DrawMesh(level._mapMesh, mapMaterial, &consts, sizeof(FrameConstants));

        // Draw floor
        consts.mvp = proj * view * glm::scale(glm::mat4{1.0f}, glm::vec3{10.0f});
        renderer.DrawMesh(level._floorMesh, groundMaterial, &consts, sizeof(FrameConstants));

        // Draw player
        //consts.mvp = proj * view * glm::translate(glm::mat4{1.0f}, playerTrans.position) * glm::rotate(glm::mat4{1.0f}, -cubeAngleRad, {0.0f, 1.0f, 0.0f}) * glm::scale(glm::mat4{1.0f}, glm::vec3{3.0f, 10.0f, 3.0f});
        //renderer.DrawMesh(cubeMesh, mapMaterial, &consts, sizeof(FrameConstants));

        // Draw doors
        auto rendeables = registry.view<Game::Transform, Game::Renderable>();
        for (auto [entity, rtransform, rmesh] : rendeables.each())
        {
            consts.mvp = proj * view * glm::translate(glm::mat4{1.0f}, rtransform.position) * glm::scale(glm::mat4{1.0f}, rtransform.scale);
            renderer.DrawMesh(cubeMesh, mapMaterial, &consts, sizeof(FrameConstants));
        }

        // Draw sprites
        renderer.Draw(6, (uint32_t)spriteModelMats.size(), spriteMaterial, nullptr, 0);

        // Hud
        auto orthoMat = glm::ortho(0.0f, (float)renderer._swapchain->GetExtent().width, (float)renderer._swapchain->GetExtent().height, 0.0f);

        float screenHeight = renderer._swapchain->GetExtent().height;
        float screenWidth = renderer._swapchain->GetExtent().width;
        float size = screenWidth / 3.0f;

        glm::vec2 weaponSize{ size, size };
        HudPushConstants hudPushConstants{orthoMat, weaponSize, {screenWidth / 2.0f, screenHeight - size / 2.0f}, 421};
        renderer.Draw(6, 1, hudMaterial, &hudPushConstants, sizeof(HudPushConstants));

        renderer.End();
    }

    return 0;
}
