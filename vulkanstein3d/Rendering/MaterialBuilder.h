#pragma once

#include "Device.h"
#include "PipelineBuilder.h"
#include "Texture.h"

#include <string>

namespace Rendering
{
class Buffer;

class Material
{
  public:
    std::shared_ptr<Pipeline> _pipeline;
    vk::DescriptorSet _descriptorSet{};
};

class MaterialBuilder
{
  public:
    static MaterialBuilder Builder();

    MaterialBuilder& SetPipeline(std::shared_ptr<Pipeline> pipeline);
    MaterialBuilder& SetTexture(std::shared_ptr<Texture> texture);
    MaterialBuilder& SetUboHack(std::shared_ptr<Buffer> ubo, std::shared_ptr<Buffer> storage);

    std::shared_ptr<Material> Build(std::shared_ptr<Device> device);

  private:
    std::shared_ptr<Device> _device;
    std::shared_ptr<Pipeline> _pipeline;
    std::shared_ptr<Texture> _texture;
    std::shared_ptr<Buffer> _ubo;
    std::shared_ptr<Buffer> _storage;
};

} // namespace Rendering
