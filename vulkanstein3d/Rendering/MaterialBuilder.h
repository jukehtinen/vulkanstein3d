#pragma once

#include "Device.h"
#include "PipelineBuilder.h"
#include "Texture.h"

#include <map>
#include <string>
#include <vector>

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
    MaterialBuilder& SetTexture(uint32_t binding, std::shared_ptr<Texture> texture);
    MaterialBuilder& SetTexture(uint32_t binding, std::vector<std::shared_ptr<Texture>> textures);
    MaterialBuilder& SetBuffer(uint32_t binding, std::shared_ptr<Buffer> buffer);

    std::shared_ptr<Material> Build(std::shared_ptr<Device> device);

  private:
    std::shared_ptr<Device> _device;
    std::shared_ptr<Pipeline> _pipeline;
    std::map<uint32_t, std::shared_ptr<Buffer>> _buffers;
    std::map<uint32_t, std::shared_ptr<Texture>> _textures;
    std::map<uint32_t, std::vector<std::shared_ptr<Texture>>> _textureVariableCounts;
};

} // namespace Rendering
