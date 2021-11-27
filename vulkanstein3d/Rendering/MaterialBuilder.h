#pragma once

#include "Device.h"
#include "PipelineBuilder.h"
#include "Texture.h"

#include <string>

namespace Rendering
{
class Material
{
  public:
    Pipeline pipeline{};
    vk::DescriptorSet _descriptorSet{};
};

class MaterialBuilder
{
  public:
    static MaterialBuilder Builder();

    MaterialBuilder& SetPipeline(const Rendering::Pipeline pipeline);
    MaterialBuilder& SetTexture(std::shared_ptr<Texture> texture);

    Material Build(std::shared_ptr<Device> device);

  private:
    std::shared_ptr<Device> _device;
    Pipeline _pipeline;
    std::shared_ptr<Texture> _texture;
};
} // namespace Rendering
