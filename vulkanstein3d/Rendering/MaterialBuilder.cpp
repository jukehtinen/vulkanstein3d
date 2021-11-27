#include "MaterialBuilder.h"

namespace Rendering
{
MaterialBuilder MaterialBuilder::Builder()
{
    MaterialBuilder builder{};
    return builder;
}

MaterialBuilder& MaterialBuilder::SetPipeline(const Rendering::Pipeline pipeline)
{
    _pipeline = pipeline;
    return *this;
}

MaterialBuilder& MaterialBuilder::SetTexture(std::shared_ptr<Texture> texture)
{
    _texture = texture;
    return *this;
}

Material MaterialBuilder::Build(std::shared_ptr<Device> device)
{
    std::vector<vk::DescriptorPoolSize> descriptorPoolSizes = {{vk::DescriptorType::eCombinedImageSampler, 10}};
    auto descriptorPool = device->Get().createDescriptorPool({{}, 1, descriptorPoolSizes}).value;

    vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo{descriptorPool, 1, &_pipeline.descriptorSetLayout};
    uint32_t descriptors = 1;
    vk::DescriptorSetVariableDescriptorCountAllocateInfo variableDescriptorCountAllocInfo{1, &descriptors};
    descriptorSetAllocateInfo.pNext = &variableDescriptorCountAllocInfo;

    auto descriptorSet = device->Get().allocateDescriptorSets(descriptorSetAllocateInfo).value.front();

    vk::DescriptorImageInfo imageInfo{_texture->_sampler, _texture->_imageView, vk::ImageLayout::eShaderReadOnlyOptimal};

    std::vector<vk::WriteDescriptorSet> writes{vk::WriteDescriptorSet{descriptorSet, 0, 0, 1, vk::DescriptorType::eCombinedImageSampler, &imageInfo, {}, {}}};

    device->Get().updateDescriptorSets(writes, {});

    return Material{
        _pipeline,
        descriptorSet};
}
} // namespace Rendering