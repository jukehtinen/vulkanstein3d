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

struct ObjectPushConstants
{
    glm::mat4 mvp{1.0f};
    float tileIndex;
    float padding0;
    float padding1;
    float padding2;
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

    auto objectPipeline = Rendering::PipelineBuilder::Builder()
                              .SetShaders("Shaders/mat_object.vert.spv", "Shaders/mat_object.frag.spv")
                              .Build(device);

    assets.AddMaterial("mat_object", Rendering::MaterialBuilder::Builder()
                                         .SetPipeline(objectPipeline)
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

    int levelIndex = 0;
    auto level = std::make_shared<Game::Level>(renderer, loaders.LoadMap((levelIndex / 10) + 1, (levelIndex % 10) + 1));

    auto cubeMesh = Game::MeshGenerator::BuildCubeMesh(renderer._device);

    std::vector<Sprite> spriteModelMats;

    auto frameConstsUbo = Rendering::Buffer::CreateUniformBuffer(renderer._device, sizeof(FrameConstantsUBO));
    auto modelStorage = Rendering::Buffer::CreateStorageBuffer(renderer._device, sizeof(Sprite) * 512);

    CreateMaterials(renderer._device, assets, frameConstsUbo, modelStorage);

    auto groundMaterial = assets.GetMaterial("mat_ground");
    auto mapMaterial = assets.GetMaterial("mat_map");
    auto spriteMaterial = assets.GetMaterial("mat_sprites");
    auto hudMaterial = assets.GetMaterial("mat_hud_sprites");
    auto objectMaterial = assets.GetMaterial("mat_object");

    auto prevTime = std::chrono::high_resolution_clock::now();
    double totalTime{};
    auto& input = App::Input::The();
    while (!glfwWindowShouldClose(window->Get()))
    {
        input.Update();
        glfwPollEvents();

        auto nowTime = std::chrono::high_resolution_clock::now();
        auto timeSpan = std::chrono::duration_cast<std::chrono::duration<double>>(nowTime - prevTime);
        prevTime = nowTime;
        auto delta = timeSpan.count();
        totalTime += delta;

        level->Update(delta);

        if (level->GetState() == Game::Level::LevelState::GoToNextLevel)
        {
            // todo, handle return from secret level (to back to proper level order)
            levelIndex++;
            level = std::make_shared<Game::Level>(renderer, loaders.LoadMap((levelIndex / 10) + 1, (levelIndex % 10) + 1));
            continue;
        }
        if (level->GetState() == Game::Level::LevelState::GoToSecretLevel)
        {
            level = std::make_shared<Game::Level>(renderer, loaders.LoadMap((levelIndex / 10) + 1, 10));
            continue;
        }

        auto& registry = level->GetRegistry();

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

        auto mousepos = input.GetMousePos();

        const auto& playerXform = registry.get<Game::Transform>(level->GetPlayerEntity());
        const auto& fpsCamera = registry.get<Game::FPSCamera>(level->GetPlayerEntity());

        auto view = glm::lookAt(playerXform.position, playerXform.position + fpsCamera.front, fpsCamera.up);
        auto proj = glm::perspective(glm::radians(65.0f), renderer._swapchain->GetExtent().width / (float)renderer._swapchain->GetExtent().height, 0.1f, 500.0f);

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
        renderer.DrawMesh(level->_mapMesh, mapMaterial, &consts, sizeof(FrameConstants));

        // Draw floor
        consts.mvp = proj * view * glm::scale(glm::mat4{1.0f}, glm::vec3{10.0f});
        renderer.DrawMesh(level->_floorMesh, groundMaterial, &consts, sizeof(FrameConstants));

        // Draw doors
        auto rendeables = registry.view<Game::Transform, Game::Renderable>();
        for (auto [entity, xform, renderable] : rendeables.each())
        {
            ObjectPushConstants opc{
                proj * view * glm::translate(glm::mat4{1.0f}, xform.position) * glm::scale(glm::mat4{1.0f}, xform.scale),
                (float)renderable.tileIndex};

            renderer.DrawMesh(cubeMesh, objectMaterial, &opc, sizeof(ObjectPushConstants));
        }

        // Draw sprites
        renderer.Draw(6, (uint32_t)spriteModelMats.size(), spriteMaterial, nullptr, 0);

        // Hud
        auto orthoMat = glm::ortho(0.0f, (float)renderer._swapchain->GetExtent().width, (float)renderer._swapchain->GetExtent().height, 0.0f);

        float screenHeight = (float)renderer._swapchain->GetExtent().height;
        float screenWidth = (float)renderer._swapchain->GetExtent().width;
        float size = screenWidth / 3.0f;

        int knife = 416;
        int pistol = 421;
        int kk = 426;
        int gt = 431;
        int current = 0;
        if (level->_currentWeapon == Game::Level::Weapon::Knife)
            current = knife;
        if (level->_currentWeapon == Game::Level::Weapon::Pistol)
            current = pistol;
        if (level->_currentWeapon == Game::Level::Weapon::MachineGun)
            current = kk;
        if (level->_currentWeapon == Game::Level::Weapon::Gatling)
            current = gt;
        glm::vec2 weaponSize{size, size};

        HudPushConstants hudPushConstants{orthoMat, weaponSize, {screenWidth / 2.0f, (screenHeight - size / 2.0f) + (size / 4.0) * level->_weaponChangeOffset}, 
            current + level->_weaponFrameOffset };
        renderer.Draw(6, 1, hudMaterial, &hudPushConstants, sizeof(HudPushConstants));

        renderer.End();
    }

    return 0;
}
