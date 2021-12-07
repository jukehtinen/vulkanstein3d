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

MaterialBuilder& MaterialBuilder::SetTexture(std::shared_ptr<Texture> texture)
{
    _texture = texture;
    return *this;
}

MaterialBuilder& MaterialBuilder::SetUboHack(std::shared_ptr<Buffer> ubo, std::shared_ptr<Buffer> storage)
{
    _ubo = ubo;
    _storage = storage;
    return *this;
}

std::shared_ptr<Material> MaterialBuilder::Build(std::shared_ptr<Device> device)
{
    if (_texture)
    {
        // hack

        std::vector<vk::DescriptorPoolSize> descriptorPoolSizes = {{vk::DescriptorType::eCombinedImageSampler, 10}};
        auto descriptorPool = device->Get().createDescriptorPool({{}, 1, descriptorPoolSizes}).value;

        if (_texture->_viewType == vk::ImageViewType::e2D)
        {
            vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo{descriptorPool, 1, &_pipeline->descriptorSetLayout};
            uint32_t descriptors = 1;
            vk::DescriptorSetVariableDescriptorCountAllocateInfo variableDescriptorCountAllocInfo{1, &descriptors};
            descriptorSetAllocateInfo.pNext = &variableDescriptorCountAllocInfo;

            auto descriptorSet = device->Get().allocateDescriptorSets(descriptorSetAllocateInfo).value.front();

            vk::DescriptorImageInfo imageInfo{_texture->_sampler, _texture->_imageView, vk::ImageLayout::eShaderReadOnlyOptimal};

            std::vector<vk::WriteDescriptorSet> writes{vk::WriteDescriptorSet{descriptorSet, 0, 0, 1, vk::DescriptorType::eCombinedImageSampler, &imageInfo, {}, {}}};

            device->Get().updateDescriptorSets(writes, {});

            return std::make_shared<Material>(_pipeline, descriptorSet);
        }
        else
        {
            vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo{descriptorPool, 1, &_pipeline->descriptorSetLayout};

            auto descriptorSet = device->Get().allocateDescriptorSets(descriptorSetAllocateInfo).value.front();

            vk::DescriptorImageInfo imageInfo{_texture->_sampler, _texture->_imageView, vk::ImageLayout::eShaderReadOnlyOptimal};

            std::vector<vk::WriteDescriptorSet> writes{vk::WriteDescriptorSet{descriptorSet, 0, 0, 1, vk::DescriptorType::eCombinedImageSampler, &imageInfo, {}, {}}};

            if (_ubo != nullptr)
            {
                vk::DescriptorBufferInfo bufferInfo1{_ubo->Get(), 0, VK_WHOLE_SIZE};
                vk::DescriptorBufferInfo bufferInfo2{_storage->Get(), 0, VK_WHOLE_SIZE};
                writes.push_back(vk::WriteDescriptorSet{ descriptorSet, 1, 0, 1, vk::DescriptorType::eUniformBuffer, {}, &bufferInfo1, {} });
                writes.push_back(vk::WriteDescriptorSet{ descriptorSet, 2, 0, 1, vk::DescriptorType::eStorageBuffer, {}, &bufferInfo2, {} });
                device->Get().updateDescriptorSets(writes, {});
            }
            else
            {
                device->Get().updateDescriptorSets(writes, {});
            }            

            return std::make_shared<Material>(_pipeline, descriptorSet);
        }
    }
    return std::make_shared<Material>(_pipeline, vk::DescriptorSet{});
}
} // namespace Rendering