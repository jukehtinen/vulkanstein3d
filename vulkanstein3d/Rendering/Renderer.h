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
    Renderer(std::shared_ptr<App::Window> window);
    ~Renderer();

    bool Begin();
    void End();

    void Draw(uint32_t vertexCount, uint32_t instances, std::shared_ptr<Rendering::Material> material, void* pushConstants, size_t pushConstantSize);
    void DrawMesh(Rendering::Mesh& mesh, std::shared_ptr<Rendering::Material> material, void* pushConstants, size_t pushConstantSize);

    std::shared_ptr<Rendering::Instance> _instance;
    std::shared_ptr<Rendering::Device> _device;
    std::shared_ptr<Rendering::Swapchain> _swapchain;

  private:
    std::shared_ptr<Rendering::Texture> _depthTexture;

    vk::CommandPool _commandPool{};
    vk::CommandBuffer _commandBuffer{};

    vk::Fence _renderFence{};
    vk::Semaphore _presentSemaphore{};
    vk::Semaphore _renderSemaphore{};

    uint32_t _imageIndex{0};
    bool _recreateSwapchain{false};

  private:
    void BeginRenderPass();
    void EndRenderPass();
    void Submit();
};
} // namespace Rendering