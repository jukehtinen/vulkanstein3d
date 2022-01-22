#include "../Common.h"

#include "Buffer.h"
#include "Texture.h"

namespace Rendering
{

Texture::Texture(std::shared_ptr<Device> device, vk::Image image, vk::ImageView imageView, vk::Sampler sampler, VmaAllocation allocation, vk::ImageViewType viewType)
    : _device(device), _image(image), _imageView(imageView), _sampler(sampler), _allocation(allocation), _viewType(viewType)
{
}

Texture::~Texture()
{
    _device->Get().destroySampler(_sampler);
    _device->Get().destroyImageView(_imageView);
    vmaDestroyImage(_device->GetAllocator(), _image, _allocation);
}

std::shared_ptr<Texture> Texture::CreateTexture(std::shared_ptr<Device> device, void* data, uint32_t width, uint32_t height, uint32_t layers)
{
    auto stagingBuffer = Buffer::CreateStagingBuffer(device, data, (width * height * 4) * layers);
    return CreateTexture(device, stagingBuffer, width, height, layers);
}

std::shared_ptr<Texture> Texture::CreateTexture(std::shared_ptr<Device> device, std::shared_ptr<Buffer> stagingBuffer, uint32_t width, uint32_t height, uint32_t layers)
{
    vk::ImageCreateInfo imageCreateInfo{};
    imageCreateInfo.setImageType(vk::ImageType::e2D);
    imageCreateInfo.setFormat(vk::Format::eR8G8B8A8Unorm);
    imageCreateInfo.setExtent(vk::Extent3D(width, height, 1));
    imageCreateInfo.setArrayLayers(layers);
    imageCreateInfo.setMipLevels(1);
    imageCreateInfo.setSamples(vk::SampleCountFlagBits::e1);
    imageCreateInfo.setTiling(vk::ImageTiling::eOptimal);
    imageCreateInfo.setUsage(vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst);

    vk::Image image{};
    VmaAllocation allocation{};
    VmaAllocationCreateInfo imageAllocInfo{};
    imageAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    vmaCreateImage(device->GetAllocator(), (VkImageCreateInfo*)&imageCreateInfo, &imageAllocInfo, (VkImage*)&image, &allocation, nullptr);

    device->RunCommandsSync([&](vk::CommandBuffer commandBuffer) {
        vk::ImageMemoryBarrier2KHR barrierToTransferDst{};
        barrierToTransferDst.setImage(image);
        barrierToTransferDst.setSubresourceRange(vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, layers});
        barrierToTransferDst.setOldLayout(vk::ImageLayout::eUndefined);
        barrierToTransferDst.setNewLayout(vk::ImageLayout::eTransferDstOptimal);
        barrierToTransferDst.setSrcStageMask(vk::PipelineStageFlagBits2KHR::eNone);
        barrierToTransferDst.setDstStageMask(vk::PipelineStageFlagBits2KHR::eTransfer);
        barrierToTransferDst.setSrcAccessMask(vk::AccessFlagBits2KHR::eNone);
        barrierToTransferDst.setDstAccessMask(vk::AccessFlagBits2KHR::eTransferWrite);
        commandBuffer.pipelineBarrier2KHR(vk::DependencyInfoKHR{{}, 0, nullptr, 0, nullptr, 1, &barrierToTransferDst});

        std::vector<vk::BufferImageCopy> bufferCopyRegions;
        for (uint32_t layer = 0; layer < layers; layer++)
        {
            uint32_t offset = static_cast<uint32_t>((stagingBuffer->GetSize() / layers) * layer);
            vk::BufferImageCopy region{offset, 0, 0, vk::ImageSubresourceLayers{vk::ImageAspectFlagBits::eColor, 0, layer, 1}, vk::Offset3D{0, 0, 0}, vk::Extent3D{width, height, 1}};
            bufferCopyRegions.push_back(region);
        }
        commandBuffer.copyBufferToImage(stagingBuffer->Get(), image, vk::ImageLayout::eTransferDstOptimal, bufferCopyRegions);

        vk::ImageMemoryBarrier2KHR barrierToShaderReadOnly{};
        barrierToShaderReadOnly.setImage(image);
        barrierToShaderReadOnly.setSubresourceRange(vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, layers});
        barrierToShaderReadOnly.setOldLayout(vk::ImageLayout::eTransferDstOptimal);
        barrierToShaderReadOnly.setNewLayout(vk::ImageLayout::eReadOnlyOptimalKHR);
        barrierToShaderReadOnly.setSrcStageMask(vk::PipelineStageFlagBits2KHR::eTransfer);
        barrierToShaderReadOnly.setDstStageMask(vk::PipelineStageFlagBits2KHR::eFragmentShader);
        barrierToShaderReadOnly.setSrcAccessMask(vk::AccessFlagBits2KHR::eTransferWrite);
        barrierToShaderReadOnly.setDstAccessMask(vk::AccessFlagBits2KHR::eShaderRead);

        commandBuffer.pipelineBarrier2KHR(vk::DependencyInfoKHR{{}, 0, nullptr, 0, nullptr, 1, &barrierToShaderReadOnly});
    });

    const vk::ImageViewCreateInfo imageViewCreateInfo{{}, image, layers == 1 ? vk::ImageViewType::e2D : vk::ImageViewType::e2DArray, imageCreateInfo.format, {}, vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, layers}};

    auto imageView = device->Get().createImageView(imageViewCreateInfo).value;

    vk::SamplerCreateInfo samplerCreateInfo{};
    samplerCreateInfo.setMagFilter(vk::Filter::eNearest);
    samplerCreateInfo.setMinFilter(vk::Filter::eNearest);
    samplerCreateInfo.setMipmapMode(vk::SamplerMipmapMode::eLinear);
    samplerCreateInfo.setAddressModeU(vk::SamplerAddressMode::eClampToEdge);
    samplerCreateInfo.setAddressModeV(vk::SamplerAddressMode::eClampToEdge);
    samplerCreateInfo.setAddressModeW(vk::SamplerAddressMode::eClampToEdge);
    samplerCreateInfo.setMinLod(0.0f);
    samplerCreateInfo.setMaxLod(1.0f);
    samplerCreateInfo.setMaxAnisotropy(1.0f);

    auto sampler = device->Get().createSampler(samplerCreateInfo).value;

    return std::make_shared<Texture>(device, image, imageView, sampler, allocation, imageViewCreateInfo.viewType);
}

std::shared_ptr<Texture> Texture::CreateDepthTexture(std::shared_ptr<Device> device, uint32_t width, uint32_t height)
{
    vk::ImageCreateInfo imageCreateInfo{};
    imageCreateInfo.setImageType(vk::ImageType::e2D);
    imageCreateInfo.setFormat(vk::Format::eD32Sfloat);
    imageCreateInfo.setExtent(vk::Extent3D(width, height, 1));
    imageCreateInfo.setArrayLayers(1);
    imageCreateInfo.setMipLevels(1);
    imageCreateInfo.setSamples(vk::SampleCountFlagBits::e1);
    imageCreateInfo.setTiling(vk::ImageTiling::eOptimal);
    imageCreateInfo.setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment);

    vk::Image image{};
    VmaAllocation allocation{};
    VmaAllocationCreateInfo imageAllocInfo{};
    imageAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    vmaCreateImage(device->GetAllocator(), (VkImageCreateInfo*)&imageCreateInfo, &imageAllocInfo, (VkImage*)&image, &allocation, nullptr);

    const vk::ImageViewCreateInfo imageViewCreateInfo{{}, image, vk::ImageViewType::e2D, imageCreateInfo.format, {}, vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1}};

    auto imageView = device->Get().createImageView(imageViewCreateInfo).value;

    return std::make_shared<Texture>(device, image, imageView, vk::Sampler{}, allocation, imageViewCreateInfo.viewType);
}

} // namespace Rendering