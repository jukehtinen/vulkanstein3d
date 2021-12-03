#pragma once

#include "Device.h"

#include <string>

namespace Rendering
{
class Device;

class Pipeline
{
  public:
    vk::Pipeline pipeline{};
    vk::PipelineLayout pipelineLayout{};
    vk::DescriptorSetLayout descriptorSetLayout{};
};

class PipelineBuilder
{
  public:
    static PipelineBuilder Builder();

    PipelineBuilder& SetShaders(const std::string& vertFile, const std::string& fragFile);
    PipelineBuilder& SetVertexInput(const std::vector<vk::VertexInputBindingDescription>& bindingDescriptions, const std::vector<vk::VertexInputAttributeDescription>& attributes);
    PipelineBuilder& SetDepthState(bool depthTest, bool depthWrite);
    PipelineBuilder& SetBlend(bool enable);
    PipelineBuilder& SetRenderpass(vk::RenderPass renderPass);
    PipelineBuilder& SetLayouts(vk::PipelineLayout layout, vk::DescriptorSetLayout descriptorLayout);
    PipelineBuilder& SetPushConstants(uint32_t size);
    PipelineBuilder& SetDynamicState(const std::vector<vk::DynamicState>& dynamicState);
    PipelineBuilder& SetRasterization(vk::CullModeFlags cullMode, vk::FrontFace frontFace);

    std::shared_ptr<Pipeline> Build(std::shared_ptr<Device> device);

  private:
    PipelineBuilder();

    vk::ShaderModule LoadShaderFile(const std::string& shaderFile);

  private:
    std::shared_ptr<Device> _device;
    vk::RenderPass _renderPass{};
    vk::PipelineLayout _pipelineLayout{};
    vk::DescriptorSetLayout _descriptorLayout{};

    std::string _vertFile;
    std::string _fragFile;

    std::vector<vk::VertexInputBindingDescription> _bindingDescriptions;
    std::vector<vk::VertexInputAttributeDescription> _vertexAttributes;
    bool _isDepthTest{true};
    bool _isDepthWrite{true};
    bool _isBlending{false};
    uint32_t _pushConstantSize{};
    vk::CullModeFlags _cullMode{vk::CullModeFlagBits::eNone};
    vk::FrontFace _frontFace{vk::FrontFace::eCounterClockwise};
    std::vector<vk::DynamicState> _dynamicState{vk::DynamicState::eViewport, vk::DynamicState::eScissor};
};
} // namespace Rendering