#include "../Common.h"

#include "Buffer.h"
#include "MaterialBuilder.h"

namespace Rendering
{
MaterialBuilder MaterialBuilder::Builder()
{
    MaterialBuilder builder{};
    return builder;
}

MaterialBuilder& MaterialBuilder::SetPipeline(std::shared_ptr<Rendering::Pipeline> pipeline)
{
    _pipeline = pipeline;
    return *this;
}

MaterialBuilder& MaterialBuilder::SetTexture(uint32_t binding, std::shared_ptr<Texture> texture)
{
    _textures[binding] = texture;
    return *this;
}

MaterialBuilder& MaterialBuilder::SetTexture(uint32_t binding, std::vector<std::shared_ptr<Texture>> textures)
{
    _textureVariableCounts[binding] = textures;
    return *this;
}

MaterialBuilder& MaterialBuilder::SetBuffer(uint32_t binding, std::shared_ptr<Buffer> buffer)
{
    _buffers[binding] = buffer;
    return *this;
}

std::shared_ptr<Material> MaterialBuilder::Build(std::shared_ptr<Device> device)
{
    // hack
    if (!_pipeline->descriptorSetLayout)
        return std::make_shared<Material>(_pipeline, vk::DescriptorSet{});

    std::vector<vk::DescriptorPoolSize> descriptorPoolSizes = {{vk::DescriptorType::eCombinedImageSampler, 10}};
    auto descriptorPool = device->Get().createDescriptorPool({{}, 1, descriptorPoolSizes}).value;

    vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo{descriptorPool, 1, &_pipeline->descriptorSetLayout};

    uint32_t descriptors = _textureVariableCounts.size();
    vk::DescriptorSetVariableDescriptorCountAllocateInfo variableDescriptorCountAllocInfo{1, &descriptors};
    if (_textureVariableCounts.size() > 0)
    {
        descriptorSetAllocateInfo.pNext = &variableDescriptorCountAllocInfo;
    }

    auto descriptorSet = device->Get().allocateDescriptorSets(descriptorSetAllocateInfo).value.front();

    for (const auto& [binding, texture] : _textures)
    {
        vk::DescriptorImageInfo imageInfo{texture->_sampler, texture->_imageView, vk::ImageLayout::eShaderReadOnlyOptimal};
        vk::WriteDescriptorSet write{descriptorSet, binding, 0, 1, vk::DescriptorType::eCombinedImageSampler, &imageInfo, {}, {}};
        device->Get().updateDescriptorSets(1, &write, 0, nullptr);
    }

    for (const auto& [binding, textures] : _textureVariableCounts)
    {
        std::vector<vk::DescriptorImageInfo> imageInfos;
        for (const auto& texture : textures)
            imageInfos.push_back(vk::DescriptorImageInfo{texture->_sampler, texture->_imageView, vk::ImageLayout::eShaderReadOnlyOptimal});

        vk::WriteDescriptorSet write{descriptorSet, binding, 0, vk::DescriptorType::eCombinedImageSampler, imageInfos, {}, {}};
        device->Get().updateDescriptorSets(1, &write, 0, nullptr);
    }

    int hack = 0;
    for (const auto& [binding, buffer] : _buffers)
    {
        vk::DescriptorBufferInfo bufferInfo{buffer->Get(), 0, VK_WHOLE_SIZE};
        vk::WriteDescriptorSet write{descriptorSet, binding, 0, 1, hack == 0 ? vk::DescriptorType::eUniformBuffer : vk::DescriptorType::eStorageBuffer, {}, &bufferInfo, {}};
        device->Get().updateDescriptorSets(1, &write, 0, nullptr);
        hack++;
    }

    return std::make_shared<Material>(_pipeline, descriptorSet);
}
} // namespace Rendering