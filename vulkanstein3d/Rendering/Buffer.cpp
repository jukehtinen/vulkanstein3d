#include "../Common.h"

#include "Buffer.h"
#include "Device.h"

namespace Rendering
{

Buffer::Buffer(std::shared_ptr<Device> device, vk::Buffer buffer, VmaAllocation allocation, size_t size)
    : _device(device), _buffer(buffer), _allocation(allocation), _size(size)
{
}

Buffer::~Buffer()
{
    vmaDestroyBuffer(_device->GetAllocator(), _buffer, _allocation);
}

std::shared_ptr<Buffer> Buffer::CreateStagingBuffer(std::shared_ptr<Device> device, void* data, size_t size)
{
    const vk::BufferCreateInfo bufferCreateInfo{{}, size, vk::BufferUsageFlagBits::eTransferSrc};

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

    vk::Buffer buffer{};
    VmaAllocation allocation{};

    vmaCreateBuffer(device->GetAllocator(), (VkBufferCreateInfo*)&bufferCreateInfo, &allocInfo, (VkBuffer*)&buffer, &allocation, nullptr);

    void* mapping = nullptr;
    vmaMapMemory(device->GetAllocator(), allocation, &mapping);
    std::memcpy(mapping, data, size);
    vmaUnmapMemory(device->GetAllocator(), allocation);

    return std::make_shared<Buffer>(device, buffer, allocation, size);
}

std::shared_ptr<Buffer> Buffer::CreateGPUBuffer(std::shared_ptr<Device> device, vk::BufferUsageFlags usage, size_t size)
{
    const vk::BufferCreateInfo bufferCreateInfo{{}, size, vk::BufferUsageFlagBits::eTransferDst | usage};

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    vk::Buffer buffer{};
    VmaAllocation allocation{};

    vmaCreateBuffer(device->GetAllocator(), (VkBufferCreateInfo*)&bufferCreateInfo, &allocInfo, (VkBuffer*)&buffer, &allocation, nullptr);

    return std::make_shared<Buffer>(device, buffer, allocation, size);
}

std::shared_ptr<Buffer> Buffer::CreateUniformBuffer(std::shared_ptr<Device> device, size_t size)
{
    const vk::BufferCreateInfo bufferCreateInfo{{}, size, vk::BufferUsageFlagBits::eUniformBuffer};

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    vk::Buffer buffer{};
    VmaAllocation allocation{};

    vmaCreateBuffer(device->GetAllocator(), (VkBufferCreateInfo*)&bufferCreateInfo, &allocInfo, (VkBuffer*)&buffer, &allocation, nullptr);

    return std::make_shared<Buffer>(device, buffer, allocation, size);
}

std::shared_ptr<Buffer> Buffer::CreateStorageBuffer(std::shared_ptr<Device> device, size_t size)
{
    const vk::BufferCreateInfo bufferCreateInfo{{}, size, vk::BufferUsageFlagBits::eStorageBuffer};

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    vk::Buffer buffer{};
    VmaAllocation allocation{};

    vmaCreateBuffer(device->GetAllocator(), (VkBufferCreateInfo*)&bufferCreateInfo, &allocInfo, (VkBuffer*)&buffer, &allocation, nullptr);

    return std::make_shared<Buffer>(device, buffer, allocation, size);
}

void Buffer::CopyTo(std::shared_ptr<Buffer> targetBuffer)
{
    _device->RunCommandsSync([&](vk::CommandBuffer cmdBuffer) {
        vk::BufferCopy bufferCopy{0, 0, _size};
        cmdBuffer.copyBuffer(_buffer, targetBuffer->Get(), 1, &bufferCopy);
    });
}

void Buffer::SetData(void* data, size_t size)
{
    void* mapping = nullptr;
    vmaMapMemory(_device->GetAllocator(), _allocation, &mapping);
    std::memcpy(mapping, data, size);
    vmaUnmapMemory(_device->GetAllocator(), _allocation);
}

} // namespace Rendering