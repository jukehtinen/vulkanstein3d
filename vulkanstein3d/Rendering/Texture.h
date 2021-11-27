#pragma once

#include "Device.h"

namespace Rendering
{
class Device;

class Texture
{
  public:
    static std::shared_ptr<Texture> CreateTexture(std::shared_ptr<Device> device, void *data, uint32_t width, uint32_t height);

    Texture(std::shared_ptr<Device> device, vk::Image image, vk::ImageView imageView, vk::Sampler sampler, VmaAllocation allocation);
    ~Texture();

    std::shared_ptr<Device> _device;
    vk::Image _image{};
    vk::ImageView _imageView{};
    vk::Sampler _sampler{};
    VmaAllocation _allocation{};
};
} // namespace Rendering