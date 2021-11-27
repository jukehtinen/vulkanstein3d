#include "Texture.h"

namespace Rendering
{

Texture::Texture(std::shared_ptr<Device> device, vk::Image image, vk::ImageView imageView, vk::Sampler sampler, VmaAllocation allocation)
    : _device(device), _image(image), _imageView(imageView), _sampler(sampler), _allocation(allocation)
{
}

Texture::~Texture()
{
    _device->Get().destroySampler(_sampler);
    _device->Get().destroyImageView(_imageView);
    vmaDestroyImage(_device->GetAllocator(), _image, _allocation);
}

std::shared_ptr<Texture> Texture::CreateTexture(std::shared_ptr<Device> device, void* data, uint32_t width, uint32_t height)
{
    const vk::BufferCreateInfo stagingBufferCreateInfo{{}, width * height * 4, vk::BufferUsageFlagBits::eTransferSrc};
    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
    vk::Buffer buffer{};
    VmaAllocation bufferAllocation{};
    vmaCreateBuffer(device->GetAllocator(), (VkBufferCreateInfo*)&stagingBufferCreateInfo, &allocInfo, (VkBuffer*)&buffer, &bufferAllocation, nullptr);

    void* mapping = nullptr;
    vmaMapMemory(device->GetAllocator(), bufferAllocation, &mapping);
    std::memcpy(mapping, data, width * height * 4);
    vmaUnmapMemory(device->GetAllocator(), bufferAllocation);

    vk::ImageCreateInfo imageCreateInfo{};
    imageCreateInfo.setImageType(vk::ImageType::e2D);
    imageCreateInfo.setFormat(vk::Format::eR8G8B8A8Unorm);
    imageCreateInfo.setExtent(vk::Extent3D(width, height, 1));
    imageCreateInfo.setArrayLayers(1);
    imageCreateInfo.setMipLevels(1);
    imageCreateInfo.setSamples(vk::SampleCountFlagBits::e1);
    imageCreateInfo.setTiling(vk::ImageTiling::eOptimal);
    imageCreateInfo.setUsage(vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst);

    vk::Image image{};
    VmaAllocation allocation{};
    VmaAllocationCreateInfo imageAllocInfo{};
    imageAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    vmaCreateImage(device->GetAllocator(), (VkImageCreateInfo*)&imageCreateInfo, &imageAllocInfo, (VkImage*)&image, &allocation, nullptr);

    vk::ImageMemoryBarrier2KHR barrierToTransferDst{};
    barrierToTransferDst.setImage(image);
    barrierToTransferDst.setSubresourceRange(vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
    barrierToTransferDst.setOldLayout(vk::ImageLayout::eUndefined);
    barrierToTransferDst.setNewLayout(vk::ImageLayout::eTransferDstOptimal);
    barrierToTransferDst.setSrcStageMask(vk::PipelineStageFlagBits2KHR::eNone);
    barrierToTransferDst.setDstStageMask(vk::PipelineStageFlagBits2KHR::eTransfer);
    barrierToTransferDst.setSrcAccessMask(vk::AccessFlagBits2KHR::eNone);
    barrierToTransferDst.setDstAccessMask(vk::AccessFlagBits2KHR::eTransferWrite);

    auto commandPool = device->Get().createCommandPool({vk::CommandPoolCreateFlagBits::eResetCommandBuffer}).value;
    auto commandBuffer = device->Get().allocateCommandBuffers({commandPool, vk::CommandBufferLevel::ePrimary, 1}).value.front();

    commandBuffer.begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
    commandBuffer.pipelineBarrier2KHR(vk::DependencyInfoKHR{{}, 0, nullptr, 0, nullptr, 1, &barrierToTransferDst});

    std::vector copyRegions{vk::BufferImageCopy{0, 0, 0, vk::ImageSubresourceLayers{vk::ImageAspectFlagBits::eColor, 0, 0, 1}, vk::Offset3D{0, 0, 0}, vk::Extent3D{width, height, 1}}};

    commandBuffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, copyRegions);

    vk::ImageMemoryBarrier2KHR barrierToShaderReadOnly{};
    barrierToShaderReadOnly.setImage(image);
    barrierToShaderReadOnly.setSubresourceRange(vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
    barrierToShaderReadOnly.setOldLayout(vk::ImageLayout::eTransferDstOptimal);
    barrierToShaderReadOnly.setNewLayout(vk::ImageLayout::eReadOnlyOptimalKHR);
    barrierToShaderReadOnly.setSrcStageMask(vk::PipelineStageFlagBits2KHR::eTransfer);
    barrierToShaderReadOnly.setDstStageMask(vk::PipelineStageFlagBits2KHR::eFragmentShader);
    barrierToShaderReadOnly.setSrcAccessMask(vk::AccessFlagBits2KHR::eTransferWrite);
    barrierToShaderReadOnly.setDstAccessMask(vk::AccessFlagBits2KHR::eShaderRead);

    commandBuffer.pipelineBarrier2KHR(vk::DependencyInfoKHR{{}, 0, nullptr, 0, nullptr, 1, &barrierToShaderReadOnly});
    commandBuffer.end();

    const vk::CommandBufferSubmitInfoKHR cmdBufferSubmit{commandBuffer};
    auto graphicsQueue = device->GetGraphicQueue();
    const vk::SubmitInfo2KHR submitInfo{{}, 0, nullptr, 1, &cmdBufferSubmit, 0, nullptr};
    graphicsQueue.submit2KHR(1, &submitInfo, {});
    graphicsQueue.waitIdle();
    device->Get().freeCommandBuffers(commandPool, 1, &commandBuffer);

    vmaDestroyBuffer(device->GetAllocator(), buffer, bufferAllocation);

    const vk::ImageViewCreateInfo imageViewCreateInfo{{}, image, vk::ImageViewType::e2D, imageCreateInfo.format, {}, vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}};

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

    return std::make_shared<Texture>(device, image, imageView, sampler, allocation);
}
} // namespace Rendering