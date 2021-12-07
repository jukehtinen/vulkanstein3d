#pragma once

namespace Rendering
{
class Device;

class Buffer
{
  public:
    static std::shared_ptr<Buffer> CreateStagingBuffer(std::shared_ptr<Device> device, void* data, size_t size);
    static std::shared_ptr<Buffer> CreateGPUBuffer(std::shared_ptr<Device> device, vk::BufferUsageFlags usage, size_t size);
    static std::shared_ptr<Buffer> CreateUniformBuffer(std::shared_ptr<Device> device, size_t size);
    static std::shared_ptr<Buffer> CreateStorageBuffer(std::shared_ptr<Device> device, size_t size);

    Buffer(std::shared_ptr<Device> device, vk::Buffer buffer, VmaAllocation allocation, size_t size);
    ~Buffer();

    vk::Buffer Get() const { return _buffer; }
    size_t GetSize() const { return _size; }
    void CopyTo(std::shared_ptr<Buffer> targetBuffer);

    void SetData(void* data, size_t size);

  private:
    std::shared_ptr<Device> _device;
    vk::Buffer _buffer{};
    VmaAllocation _allocation{};
    size_t _size{};
};
} // namespace Rendering
